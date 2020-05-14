#pragma once

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "structs.hpp"

using namespace std;

struct TimeCycle {
    int LD = 3, JUMP = 1, ADD = 3, SUB = 3, MUL = 4, DIV = 4;
};

const int fuNum = 7;
const int aNum = 6;
const int mNum = 3;
const int lbNum = 3;
const int amNum = 9;
const int lbamNum = 12;
const int lbmNum = 6;

class Tomasulo {
   private:
    vector<string> nel;
    vector<Code> codes;
    vector<pair<int, ReservationStations*>> cdb;
    // string fu_writeResult[fuNum];
    int clock = 0;
    int current = 0;
    Register regs[32];
    ReservationStations RS[12];
    ReservationStations* lb = (ReservationStations*)RS;
    ReservationStations* mrs = lb + 3;
    ReservationStations* ars = mrs + 3;
    vector<CodeState> codeState;
    const TimeCycle time;
    bool done = false;
    int fus[fuNum];
    vector<Log> logs;

    string rs2str(ReservationStations* rs) {
        switch (rs - RS) {
            case 0:
                return "LB 1";
            case 1:
                return "LB 2";
            case 2:
                return "LB 3";
            case 3:
                return "Mrs 1";
            case 4:
                return "Mrs 2";
            case 5:
                return "Mrs 3";
            case 6:
                return "Ars 1";
            case 7:
                return "Ars 2";
            case 8:
                return "Ars 3";
            case 9:
                return "Ars 4";
            case 10:
                return "Ars 5";
            case 11:
                return "Ars 6";
        }
        return "";
    }

    void notify(int regInd, ReservationStations* rs, int imm) {
        auto amrs = mrs;
        for (int i = 0; i < amNum; ++i) {
            // ars + mrs
            if (amrs[i].Qj == rs || amrs[i].Qk == rs) {
                if (amrs[i].Qj == rs) {
                    amrs[i].Vj = imm;
                    amrs[i].Qj = nullptr;
                }
                if (amrs[i].Qk == rs) {
                    amrs[i].Vk = imm;
                    amrs[i].Qk = nullptr;
                }
                if (amrs[i].op == "DIV" && amrs[i].Vk == 0 &&
                    amrs[i].Qk == nullptr) {
                    amrs[i].remain = 1;  // DIV 0
                }
                amrs[i].ready_time = clock;
            }
        }
    }

    void finish(ReservationStations* rs, int op) {
        if (op == 0) {
            fus[rs->fu] = -1;
            rs->fu = FU::None;
            return;
        }
        if (op == 1) {
            int line = rs->line;
            // update reg
            int regInd = codes[line].reg1;
            if (rs->op != "JUMP") {
                int imm = 0;
                if (rs->op == "LD") {
                    imm = rs->imm;
                } else if (rs->op == "ADD") {
                    imm = rs->Vj + rs->Vk;
                } else if (rs->op == "SUB") {
                    imm = rs->Vj - rs->Vk;
                } else if (rs->op == "MUL") {
                    imm = rs->Vj * rs->Vk;
                } else if (rs->op == "DIV") {
                    if (rs->Vk == 0) {
                        imm = rs->Vj;
                    } else {
                        imm = rs->Vj / rs->Vk;
                    }
                }
                if (regs[regInd].rs == rs) {
                    regs[regInd].stat = imm;
                    regs[regInd].rs = nullptr;
                }
                if (regs[regInd].update < rs->issue_time) {
                    regs[regInd].val = imm;
                    regs[regInd].update = rs->issue_time;
                }
                notify(regInd, rs, imm);
            } else {  // JUMP
                if (regs[regInd].val == codes[line].imm1) {
                    current += codes[line].imm2;
                } else {
                    current += 1;
                }
            }
            rs->line = -1;
            rs->busy = false;
            rs->wait = true;
            rs->remain = -1;
            rs->op = "";
            rs->Vj = rs->Vk = rs->imm = 0;
            codeState[line].rs = nullptr;
        }
    }

    void clear_cdb() {
        for (auto info : cdb) {
            codeState[info.first].writeResult = clock;
            if (logs[info.first].issue == -1) {
                logs[info.first].issue = codeState[info.first].issue;
                logs[info.first].execComp = codeState[info.first].execComp;
                logs[info.first].writeResult =
                    codeState[info.first].writeResult;
            }
            finish(info.second, 0);
            finish(info.second, 1);
        }
        cdb.clear();
    }

    void update() {
        for (int i = 0; i < lbamNum; ++i) {
            if (RS[i].fu != FU::None &&
                (RS[i].op == "LD" ||
                 (RS[i].Qj == nullptr && RS[i].Qk == nullptr))) {
                if (RS[i].wait) {
                    // cout << RS[i].line << " " << clock << endl;
                    // cout << clock << endl;
                    RS[i].wait = false;
                    continue;
                }
                if (RS[i].remain > 0) {
                    RS[i].remain -= 1;
                }
            } else if (RS[i].fu != FU::None) {
                // RS[i].wait = false;
            }
        }
        for (int i = 0; i < lbamNum; ++i) {
            if (RS[i].fu != FU::None &&
                (RS[i].op == "LD" ||
                 (RS[i].Qj == nullptr && RS[i].Qk == nullptr))) {
                if (RS[i].remain == 0) {
                    int line = RS[i].line;
                    codeState[line].execComp = clock;
                    // finish(&RS[i], 0);
                    cdb.push_back(make_pair(line, &RS[i]));
                }
            }
        }
    }

    FU get_fu(string op) {
        if (op == "LD") {
            if (fus[FU::Load1] == -1) {
                return FU::Load1;
            }
            if (fus[FU::Load2] == -1) {
                return FU::Load2;
            }
        } else if (op == "ADD" || op == "SUB" || op == "JUMP") {
            if (fus[FU::Add1] == -1) {
                return FU::Add1;
            }
            if (fus[FU::Add2] == -1) {
                return FU::Add2;
            }
            if (fus[FU::Add3] == -1) {
                return FU::Add3;
            }
        } else if (op == "MUL" || op == "DIV") {
            if (fus[FU::Mult1] == -1) {
                return FU::Mult1;
            }
            if (fus[FU::Mult2] == -1) {
                return FU::Mult2;
            }
        }
        return FU::None;
    }

    void assign(ReservationStations* rs, FU fu) {
        rs->fu = fu;
        rs->wait = true;
        fus[fu] = rs->line;
    }

    void lookup_fu() {
        // auto lbmrs = lb;
        // for (int i = 0; i < lbmNum; ++i) {
        //     if (lbmrs[i].remain > 0 && lbmrs[i].fu == FU::None &&
        //         (lbmrs[i].op == "LD" ||
        //          (lbmrs[i].Qj == nullptr && lbmrs[i].Qk == nullptr))) {
        //         FU fu = get_fu(lbmrs[i].op);
        //         if (fu != FU::None) {
        //             assign(&lbmrs[i], fu);
        //         }
        //     }
        // }

        vector<ReservationStations*> wait_list;
        vector<pair<int, int>> wait_list_time;
        vector<FU> available_fu;
        vector<int> index;
        ReservationStations* rs = ars;
        for (int i = 0; i < aNum; ++i) {
            if (rs[i].remain > 0 && rs[i].fu == FU::None &&
                rs[i].Qj == nullptr && rs[i].Qk == nullptr) {
                wait_list.push_back(&rs[i]);
                wait_list_time.push_back(
                    make_pair(rs[i].ready_time, rs[i].issue_time));
            }
        }
        index = argsort(wait_list_time);
        for (int i = FU::Add1; i <= FU::Add3; ++i) {
            if (fus[i] == -1) {
                available_fu.push_back((FU)i);
            }
        }
        int n = min(available_fu.size(), index.size());
        for (int i = 0; i < n; ++i) {
            assign(wait_list[index[i]], available_fu[i]);
        }

        wait_list.clear();
        wait_list_time.clear();
        available_fu.clear();
        index.clear();
        rs = mrs;
        for (int i = 0; i < mNum; ++i) {
            if (rs[i].remain > 0 && rs[i].fu == FU::None &&
                rs[i].Qj == nullptr && rs[i].Qk == nullptr) {
                wait_list.push_back(&rs[i]);
                wait_list_time.push_back(
                    make_pair(rs[i].ready_time, rs[i].issue_time));
            }
        }
        index = argsort(wait_list_time);
        for (int i = FU::Mult1; i <= FU::Mult2; ++i) {
            if (fus[i] == -1) {
                available_fu.push_back((FU)i);
            }
        }
        n = min(available_fu.size(), index.size());
        for (int i = 0; i < n; ++i) {
            assign(wait_list[index[i]], available_fu[i]);
        }

        wait_list.clear();
        wait_list_time.clear();
        available_fu.clear();
        index.clear();
        rs = lb;
        for (int i = 0; i < lbNum; ++i) {
            if (rs[i].remain > 0 && rs[i].fu == FU::None &&
                rs[i].Qj == nullptr && rs[i].Qk == nullptr) {
                wait_list.push_back(&rs[i]);
                wait_list_time.push_back(
                    make_pair(rs[i].ready_time, rs[i].issue_time));
            }
        }
        index = argsort(wait_list_time);
        for (int i = FU::Load1; i <= FU::Load2; ++i) {
            if (fus[i] == -1) {
                available_fu.push_back((FU)i);
            }
        }
        n = min(available_fu.size(), index.size());
        for (int i = 0; i < n; ++i) {
            assign(wait_list[index[i]], available_fu[i]);
        }
    }

    ReservationStations* get_rs(string op) {
        ReservationStations* rs = nullptr;
        int len = -1;
        if (op == "LD") {
            rs = lb;
            len = 3;
        } else if (op == "ADD" || op == "SUB" || op == "JUMP") {
            rs = ars;
            len = 6;
        } else if (op == "MUL" || op == "DIV") {
            rs = mrs;
            len = 3;
        } else {
            return nullptr;
        }
        for (int i = 0; i < len; ++i) {
            if (!rs[i].busy) {
                return &rs[i];
            }
        }
        return nullptr;
    }

    void issue() {
        if (current >= codes.size()) {
            return;
        }

        // check current inst
        string op = codes[current].op;
        if (op == "JUMP" &&
            codeState[current].rs != nullptr) {  // jump not finish
            return;
        }
        ReservationStations* rs = get_rs(op);
        if (rs == nullptr) {
            return;
        }

        // reset code status
        codeState[current].rs = rs;
        codeState[current].execComp = -1;
        codeState[current].writeResult = -1;
        codeState[current].issue = clock;

        // reset rs status
        rs->busy = true;
        rs->issue_time = rs->ready_time = clock;
        rs->op = op;
        rs->line = current;
        if (op == "LD") {
            rs->remain = time.LD;
        } else if (op == "ADD" || op == "SUB") {
            rs->remain = time.ADD;
        } else if (op == "MUL" || op == "DIV") {
            rs->remain = time.MUL;
        } else if (op == "JUMP") {
            rs->remain = time.JUMP;
        }

        if (op == "LD") {  // imm
            rs->imm = codes[current].imm1;
        } else {  // Vj Vk Qj Qk
            int regInd = -1;
            if (op == "JUMP") {
                regInd = codes[current].reg1;
            } else {
                regInd = codes[current].reg2;
            }
            if (regs[regInd].rs != nullptr) {
                rs->Vj = 0;
                rs->Qj = regs[regInd].rs;
            } else {
                rs->Vj = regs[regInd].stat;
                rs->Qj = nullptr;
            }
            if (op != "JUMP") {
                regInd = codes[current].reg3;
                if (regs[regInd].rs != nullptr) {
                    rs->Vk = 0;
                    rs->Qk = regs[regInd].rs;
                } else {
                    rs->Vk = regs[regInd].stat;
                    rs->Qk = nullptr;
                }
            }
        }

        if (op != "JUMP") {
            regs[codes[current].reg1].rs = rs;
            current += 1;  // calc next inst
        }
    }

    void check_done() {
        if (current < codes.size()) {
            return;
        }
        done = true;
        for (int i = 0; i < lbamNum; ++i) {
            if (RS[i].busy) {
                done = false;
                return;
            }
        }
        for (int i = 0; i < fuNum; ++i) {
            if (fus[i] >= 0) {
                done = false;
                return;
            }
        }
    }

    // void write_result() {
    //     for (int i = 0; i < lbamNum; ++i) {
    //         if (RS[i].state == RSState::writeResult) {
    //             RS[i].state = RSState::unuse;
    //             RS[i].busy = false;
    //             RS[i].remain = -1;
    //             RS[i].op = "";
    //         } else if (RS[i].state == RSState::execComp) {
    //             RS[i].state = RSState::writeResult;
    //         }
    //     }
    // }

    void step() {
        if (done)
            return;
        clock += 1;
        clear_cdb();
        // write_result();
        issue();
        lookup_fu();
        update();
        check_done();
    }

   public:
    Tomasulo() {
        for (int i = 0; i < fuNum; ++i)
            fus[i] = -1;
    }

    void reset() {
        current = 0;
        clock = 0;
        cdb.clear();
        done = false;

        // rs
        auto lbms = lb;
        for (int i = 0; i < lbamNum; ++i) {
            lbms[i].clear();
        }

        // reg
        for (int i = 0; i < 32; ++i) {
            regs[i].clear();
        }

        // fu
        for (int i = 0; i < fuNum; ++i) {
            fus[i] = -1;
        }
    }

    void run(int cnt) {
        if (cnt < 0) {
            while (!done) {
                step();
            }
        } else {
            for (int i = 0; i < cnt; ++i) {
                step();
            }
        }
    }

    int str2reg(string reg) { return atoi(reg.c_str() + 1); }

    int str2imm(string imm) {
        stringstream ss;
        unsigned int x;
        ss << imm;
        ss >> hex >> x;
        return (int)x;
    }

    void set_nel(string data) {
        reset();
        nel.clear();
        codes.clear();
        codeState.clear();
        logs.clear();
        split(data, nel, "\n");
        codes.resize(nel.size());
        codeState.resize(nel.size());
        logs.resize(nel.size());
        for (int i = 0, l = nel.size(); i < l; ++i) {
            vector<string> s;
            split(nel[i], s, ",");
            if (s.size() == 0) {
                continue;
            }
            Code code;
            if (s[0] == "LD") {
                code.op = s[0];
                code.reg1 = str2reg(s[1]);
                code.imm1 = str2imm(s[2]);
            } else if (s[0] == "JUMP") {
                code.op = s[0];
                code.imm1 = str2imm(s[1]);
                code.reg1 = str2reg(s[2]);
                code.imm2 = str2imm(s[3]);
            } else if (s[0] == "ADD" || s[0] == "SUB" || s[0] == "MUL" ||
                       s[0] == "DIV") {
                code.op = s[0];
                code.reg1 = str2reg(s[1]);
                code.reg2 = str2reg(s[2]);
                code.reg3 = str2reg(s[3]);
            }
            if (code.op.size() != 0) {
                codes[i] = code;
            }
        }
    }

    void print_amrs() {
        cout << "## 保留站状态" << endl << endl;
        cout << "| Name | Busy | Op | Vj | Vk | Qj | Qk |" << endl;
        cout << "| - | - | - | - | - | - | - |" << endl;
        for (int i = 0; i < 6; ++i) {
            if (ars[i].busy) {
                printf("| %s | %s | %s | %s | %s | %s | %s |\n",
                       rs2str(&ars[i]).c_str(), "Yes", ars[i].op.c_str(),
                       ars[i].Qj == nullptr ? to_string(ars[i].Vj).c_str() : "",
                       (ars[i].Qk == nullptr && ars[i].op != "JUMP")
                           ? to_string(ars[i].Vk).c_str()
                           : "",
                       ars[i].Qj == nullptr ? "" : rs2str(ars[i].Qj).c_str(),
                       ars[i].Qk == nullptr ? "" : rs2str(ars[i].Qk).c_str());
            } else {
                printf("| %s | %s | %s | %s | %s | %s | %s |\n",
                       rs2str(&ars[i]).c_str(), "No", "", "", "", "", "");
            }
        }
        for (int i = 0; i < 3; ++i) {
            if (mrs[i].busy) {
                printf("| %s | %s | %s | %s | %s | %s | %s |\n",
                       rs2str(&mrs[i]).c_str(), "Yes", mrs[i].op.c_str(),
                       mrs[i].Qj == nullptr ? to_string(mrs[i].Vj).c_str() : "",
                       mrs[i].Qk == nullptr ? to_string(mrs[i].Vk).c_str() : "",
                       mrs[i].Qj == nullptr ? "" : rs2str(mrs[i].Qj).c_str(),
                       mrs[i].Qk == nullptr ? "" : rs2str(mrs[i].Qk).c_str());
            } else {
                printf("| %s | %s | %s | %s | %s | %s | %s |\n",
                       rs2str(&mrs[i]).c_str(), "No", "", "", "", "", "");
            }
        }
        cout << endl;
    }

    void print_lb() {
        cout << "## Load Buffer 状态" << endl << endl;
        cout << "| Name | Busy | Imm |" << endl;
        cout << "| - | - | - |" << endl;
        for (int i = 0; i < 3; ++i) {
            printf("| %s | %s | %s |\n", rs2str(&lb[i]).c_str(),
                   lb[i].busy ? "Yes" : "No",
                   lb[i].busy ? to_string(lb[i].imm).c_str() : "");
        }
        cout << endl;
    }

    void print_clock() { cout << "# Cycle " << clock << endl << endl; }

    bool has_done() { return done; }

    void print_log() {
        for (auto i : logs) {
            printf("%d %d %d\n", i.issue, i.execComp, i.writeResult);
        }
    }

    void print_debug() {}

    ReservationStations* fu2rs(FU fu) {
        for (int i = 0; i < lbamNum; ++i) {
            if (RS[i].fu == fu) {
                return &RS[i];
            }
        }
        return nullptr;
    }

    void print_fu() {
        cout << "## 运算部件状态" << endl << endl;
        cout << "| 部件名称 | 当前执行指令 | 当前还剩几个周期 |" << endl;
        cout << "| - | - | - |" << endl;
        for (int i = 0; i < fuNum; ++i) {
            if (fus[i] >= 0) {
                printf("| %s | %s | %d |\n", fu2str((FU)i).c_str(),
                       codes[fus[i]].op.c_str(), fu2rs((FU)i)->remain);
            }
            //  else if (fu_writeResult[i] != "") {
            //     printf("| %s | %s | %d |\n", fu2str((FU)i).c_str(),
            //            fu_writeResult[i].c_str(), 0);
            //     fu_writeResult[i] = "";
            // }
            else {
                printf("| %s | %s | %s |\n", fu2str((FU)i).c_str(), " ", "");
            }
        }
        cout << endl;
    }

    void print_reg() {
        cout << "## 寄存器状态" << endl << endl;
        for (int i = 0; i < 32; ++i) {
            if (regs[i].rs != nullptr) {
                cout << "R" << i << ": " << rs2str(regs[i].rs) << endl;
            }
        }
        cout << endl;
    }
};
