#pragma once

#include <string>

using namespace std;

struct Code {
    string op;
    int reg1, reg2, reg3;
    int imm1, imm2;

    void clear() {
        op = "";
        reg1 = reg2 = reg3 = imm1 = imm2;
    }

    Code() { clear(); }
};

enum FU { Add1, Add2, Add3, Mult1, Mult2, Load1, Load2, None };

string fu2str(FU fu) {
    switch (fu) {
        case Add1:
            return "Add1";
        case Add2:
            return "Add2";
        case Add3:
            return "Add3";
        case Mult1:
            return "Mult1";
        case Mult2:
            return "Mult2";
        case Load1:
            return "Load1";
        case Load2:
            return "Load2";
        default:
            return "";
    }
}

void split(const string& s,
           vector<string>& tokens,
           const string& delimiters = " ") {
    string::size_type lastPos = s.find_first_not_of(delimiters, 0);
    string::size_type pos = s.find_first_of(delimiters, lastPos);
    while (string::npos != pos || string::npos != lastPos) {
        tokens.push_back(s.substr(lastPos, pos - lastPos));
        lastPos = s.find_first_not_of(delimiters, pos);
        pos = s.find_first_of(delimiters, lastPos);
    }
}

template <typename T>
vector<int> argsort(const vector<T>& v) {
    int Len = v.size();
    vector<int> idx(Len, 0);
    for (int i = 0; i < Len; ++i) {
        idx[i] = i;
    }
    sort(idx.begin(), idx.end(),
         [&v](int i1, int i2) { return v[i1] < v[i2]; });
    return idx;
}

struct ReservationStation {
    bool busy;
    int remain;
    FU fu;
    string op;
    int Vj, Vk;
    int imm;
    ReservationStation* Qj;
    ReservationStation* Qk;
    int issue_time;
    int line;
    bool wait;
    int ready_time;

    void clear() {
        busy = false;
        remain = -1;
        fu = FU::None;
        op = "";
        Vj = Vk = imm = 0;
        Qj = Qk = nullptr;
        issue_time = ready_time = 0;
        line = -1;
        wait = false;
    }

    ReservationStation() { clear(); }
};

struct Register {
    ReservationStation* rs;
    int val, stat, update;

    void clear() {
        rs = nullptr;
        val = stat = update = 0;
    }

    Register() { clear(); }
};

struct CodeState {
    int issue, execComp, writeResult;
    ReservationStation* rs;

    void clear() {
        issue = execComp = writeResult = -1;
        rs = nullptr;
    }

    CodeState() { clear(); }
};

struct Log {
    int issue, execComp, writeResult;

    void clear() { issue = execComp = writeResult = -1; }

    Log() { clear(); }
};
