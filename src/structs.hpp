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

enum FU { Load1, Load2, Add1, Add2, Add3, Mult1, Mult2, None };

string fu2str(FU fu) {
    switch (fu) {
        case Load1:
            return "Load1";
        case Load2:
            return "Load2";
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
    std::sort(idx.begin(), idx.end(),
              [&v](int i1, int i2) { return v[i1] < v[i2]; });
    return idx;
}

enum RSState { issue, waitExec, exec, execComp, writeResult, unuse };

struct ReservationStations {
    RSState state;
    bool busy;
    int remain;
    FU fu;
    string op;
    int Vj, Vk;
    int imm;
    ReservationStations* Qj;
    ReservationStations* Qk;
    int time;
    int line;

    void clear() {
        state = RSState::unuse;
        busy = false;
        remain = -1;
        fu = FU::None;
        op = "";
        Vj = Vk = imm = 0;
        Qj = Qk = nullptr;
        time = 0;
        line = -1;
    }

    ReservationStations() { clear(); }
};

struct Register {
    ReservationStations* rs;
    int val, stat, update;

    void clear() {
        rs = nullptr;
        val = stat = update = 0;
    }

    Register() { clear(); }
};

struct CodeState {
    int issue, execComp, writeResult;
    ReservationStations* rs;

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