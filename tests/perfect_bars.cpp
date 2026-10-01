// Standalone regression test: compile with the x64 MSVC developer environment.
// Uses the production calculation/renderers with synthetic player data and
// captured draw calls, without loading or modifying a game process.
#include "../strafe analyzer/Analyzer/Features/TickHistory.cpp"
#include "../strafe analyzer/SDK+/Netvars.cpp"
#include "../strafe analyzer/Analyzer/Features/StrafeTrainer.cpp"
#include <stdexcept>

c_menu g_menu;
std::mutex History::mutex;
std::vector<TickData> History::tickHist;
std::vector<StrafeData> History::strafeHist;
std::vector<TickData> History::recordedRun;
namespace Interfaces {
    IBaseClientDLL* client = nullptr;
    IVEngineClient* engine = nullptr;
    CGlobalVars* globals = nullptr;
    ConVar* sv_airaccelerate = nullptr;
}
struct DrawCall { int x, y, w, h; color c; bool isLine = false; };
std::vector<DrawCall> calls;
namespace render {
    void filled_rect(int x, int y, int w, int h, color c) noexcept { calls.push_back({x,y,w,h,c}); }
    void line(int x, int y, int x2, int y2, color c) noexcept { calls.push_back({x,y,x2-x,y2-y,c,true}); }
    void rect(int, int, int, int, color) noexcept {}
}
void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
ClientClass* classHead;
ClientClass* getClasses(IBaseClientDLL*) { return classHead; }
void screenSize(IVEngineClient*, int& w, int& h) { w = 1280; h = 720; }
alignas(8) unsigned char convarParent[128]{};
ConVar* findVar(IConVar*, const char* name) {
    check(std::string(name) == "sv_airaccelerate", "ConVar lookup name");
    return reinterpret_cast<ConVar*>(convarParent);
}
int main() {
    try {
        // Shared nested tables must not corrupt an owning player's offsets.
        RecvProp fields[3]{};
        fields[0].m_pVarName = const_cast<char*>("m_vecVelocity[0]"); fields[0].m_Offset = 0x148;
        fields[1].m_pVarName = const_cast<char*>("m_fFlags"); fields[1].m_Offset = 0x440;
        fields[2].m_pVarName = const_cast<char*>("m_vecOrigin"); fields[2].m_Offset = 0x428;
        RecvTable base{}; base.m_pProps = fields; base.m_nProps = 3;
        base.m_pNetTableName = const_cast<char*>("DT_BasePlayer");
        RecvProp nested{}; nested.m_pVarName = const_cast<char*>("baseclass"); nested.m_pDataTable = &base;
        RecvTable cs{}; cs.m_pProps = &nested; cs.m_nProps = 1; cs.m_pNetTableName = const_cast<char*>("DT_CSPlayer");
        RecvProp shifted = nested; shifted.m_Offset = 0x80;
        RecvTable other{}; other.m_pProps = &shifted; other.m_nProps = 1; other.m_pNetTableName = const_cast<char*>("DT_Other");
        ClientClass classes[3]{};
        classes[0].m_pRecvTable = &base; classes[0].m_pNext = &classes[1];
        classes[1].m_pRecvTable = &cs; classes[1].m_pNext = &classes[2];
        classes[2].m_pRecvTable = &other; classHead = classes;
        void* clientTable[9]{}; clientTable[8] = reinterpret_cast<void*>(&getClasses);
        void** clientObject = clientTable; Interfaces::client = reinterpret_cast<IBaseClientDLL*>(&clientObject);
        Indices::get_client_classes = 8;
        check(netvar_manager::get_net_var(fnv::hash("DT_BasePlayer"), fnv::hash("m_fFlags")) == 0x440, "Nested table overwrote flags");
        check(netvar_manager::get_net_var(fnv::hash("DT_Missing"), fnv::hash("m_fFlags")) == 0, "Unrelated table fallback");

        // The registered parent, rather than a client copy, owns the current value.
        alignas(8) unsigned char convarChild[128]{};
        auto* parent = reinterpret_cast<ConVar*>(convarParent);
        *reinterpret_cast<ConVar**>(convarChild + 0x38) = parent;
        *reinterpret_cast<ConVar**>(convarParent + 0x38) = parent;
        *reinterpret_cast<float*>(convarParent + 0x54) = 100.0f;
        *reinterpret_cast<float*>(convarChild + 0x54) = 0.0001f;
        Interfaces::sv_airaccelerate = reinterpret_cast<ConVar*>(convarChild);
        check(Interfaces::sv_airaccelerate->get_float() == 100.0f, "Replicated ConVar parent");
        void* cvarTable[13]{}; cvarTable[12] = reinterpret_cast<void*>(&findVar);
        void** cvarObject = cvarTable;
        check(reinterpret_cast<IConVar*>(&cvarObject)->get_convar("sv_airaccelerate") == parent, "FindVar slot");

        alignas(8) unsigned char playerBytes[0x600]{};
        player = reinterpret_cast<player_t*>(playerBytes);
        *reinterpret_cast<vec3_t*>(playerBytes + 0x148) = {250.0f, 0.0f, 0.0f};
        *reinterpret_cast<int*>(playerBytes + 0x440) = fl_onground;
        CGlobalVars globals{}; globals.interval_per_tick = 1.0f / 66.0f; Interfaces::globals = &globals;
        CUserCmd cmd{};
        TickHistory::Update(&cmd);
        check(History::tickHist.back().perfDeltaYaw == 1.18, "Ground prespeed target missing");
        *reinterpret_cast<int*>(playerBytes + 0x440) = 0;
        TickHistory::Update(&cmd);
        check(std::abs(History::tickHist.back().perfDeltaYaw - RAD2DEG(std::atan2(30.0, 250.0))) < 0.0001, "Air target calculation");

        void* engineTable[6]{}; engineTable[5] = reinterpret_cast<void*>(&screenSize);
        void** engineObject = engineTable; Interfaces::engine = reinterpret_cast<IVEngineClient*>(&engineObject);
        Indices::get_screen_size = 5;
        g_menu.strafetrainer.enabled = true;
        g_menu.strafetrainer.dataHeight = 1;
        for (double target : {1.18, 0.01, 25.0}) {
            for (int mode = 0; mode < 4; ++mode) {
                for (double delta : {target, target * 2.0}) {
                    TickData tick{}; tick.perfDeltaYaw = target; tick.deltaYaw = delta;
                    History::tickHist.assign(3, tick);
                    g_menu.strafetrainer.graphType = mode;
                    calls.clear(); StrafeTrainer::Paint();
                    check(!calls.empty(), "Trainer did not draw");
                    if (mode == 0) {
                        for (const auto& call : calls) check(!call.isLine, "Filled graph drew a line overlay");
                    }
                    const auto& guide = mode == 0 ? calls.front() : calls.back();
                    check(guide.c.r == 255 && guide.c.g == 255 && guide.c.b == 255 && guide.c.a == 120, "Player painted over perfect guide");
                    check(guide.y >= 0 && guide.y < 720, "Perfect guide off screen");
                    if (mode < 2) check(guide.y < g_menu.strafetrainer.yOffset, "Small target rounded to zero");
                }
            }
        }
        // Filled targets are drawn first and only for airborne samples.
        g_menu.strafetrainer.graphType = 0;
        TickData ground{}; ground.posType = PositionType::GROUND;
        ground.perfDeltaYaw = 1.18; ground.deltaYaw = 2.0;
        TickData air = ground; air.posType = PositionType::AIR;
        History::tickHist = {ground, air, ground};
        calls.clear(); StrafeTrainer::Paint();
        check(calls.size() == 4, "Filled graph should draw one air target and three player bars");
        check(calls.front().c.a == 120, "Perfect fill must be underneath player fill");
        for (std::size_t i = 1; i < calls.size(); ++i)
            check(calls[i].c.a == 255, "Player fill must be on top");
        History::tickHist.assign(3, ground);
        calls.clear(); StrafeTrainer::Paint();
        check(calls.size() == 3, "Ground samples must not draw perfect fills");
        for (const auto& call : calls) check(call.c.a == 255, "Ground perfect fill visible");
        calls.clear(); History::tickHist.clear(); StrafeTrainer::Paint();
        check(calls.empty(), "Empty history drew stale guides");
        std::cout << "PASS: all four trainer styles, overlap, tiny/large targets, 720p placement, ground/air targets, ConVar parent and netvar isolation\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n'; return 1;
    }
}
