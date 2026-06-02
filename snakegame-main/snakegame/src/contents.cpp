// ────────────────────────────────────────────────
// contents.cpp
// 스코어보드, 미션 달성 여부, 아이템·게이트 타이머
// 실시간 표시를 담당하는 Contents 클래스 구현
// ────────────────────────────────────────────────

#include "contents.hpp"
#include <ncurses.h>
#include <cstdio>

// ── 상태 갱신 ────────────────────────────────────────────────────────────────

void Contents::set_status(int cur_len, int max_len, int growth, int poison, int gate) {
    cur_length = cur_len;
    max_length = max_len;
    growth_cnt = growth;
    poison_cnt = poison;
    gate_cnt   = gate;
}

void Contents::set_mission(int m_len, int m_growth, int m_poison, int m_gate,
                           bool len_ok, bool growth_ok, bool poison_ok, bool gate_ok) {
    mission_len    = m_len;
    mission_growth = m_growth;
    mission_poison = m_poison;
    mission_gate   = m_gate;
    mission_len_ok    = len_ok;
    mission_growth_ok = growth_ok;
    mission_poison_ok = poison_ok;
    mission_gate_ok   = gate_ok;
}

void Contents::set_stage(int stage) {
    current_stage = stage;
}

// immune_remain, slow_remain: 남은 시간(초). -1이면 비활성
// gate_remain: 게이트 재생성까지 남은 시간(초)
void Contents::set_item_timer(double immune_remain_in, double slow_remain_in, double gate_remain_in) {
    immune_remain = immune_remain_in;
    slow_remain   = slow_remain_in;
    gate_remain   = gate_remain_in;
}

// ── 스코어보드 출력 ──────────────────────────────────────────────────────────

void Contents::draw(int x, int y) const {
    attron(COLOR_PAIR(9)); // 흰 배경 + 검정 글자

    // ── 스테이지 표시
    mvprintw(y,      x, "[ Stage %d ]    ", current_stage);

    // ── Score Board
    mvprintw(y + 2,  x, "Score Board     ");
    mvprintw(y + 3,  x, "B: %d / %d      ", cur_length, max_length);
    mvprintw(y + 4,  x, "+: %d           ", growth_cnt);
    mvprintw(y + 5,  x, "-: %d           ", poison_cnt);
    mvprintw(y + 6,  x, "Gate: %d        ", gate_cnt);

    // ── Mission
    mvprintw(y + 8,  x, "Mission         ");
    mvprintw(y + 9,  x, "B   : %d (%s)   ", mission_len,    mission_len_ok    ? "v" : " ");
    mvprintw(y + 10, x, "+   : %d (%s)   ", mission_growth, mission_growth_ok ? "v" : " ");
    mvprintw(y + 11, x, "-   : %d (%s)   ", mission_poison, mission_poison_ok ? "v" : " ");
    mvprintw(y + 12, x, "Gate: %d (%s)   ", mission_gate,   mission_gate_ok   ? "v" : " ");

    // ── 게이트 재생성 타이머
    mvprintw(y + 14, x, "----------------");
    mvprintw(y + 15, x, "Gate in: %.1fs  ", gate_remain);

    // ── 아이템 안내 및 타이머
    // 스테이지 1·2: 슬로우 아이템 / 스테이지 3·4: 무적 아이템
    mvprintw(y + 17, x, "Items           ");
    if (current_stage == 1 || current_stage == 2) {
        // 슬로우 아이템만 표시
        mvprintw(y + 18, x, "Yellow : Slow   ");
        if (slow_remain >= 0.0) {
            attron(A_BOLD);
            mvprintw(y + 19, x, "Slow : %.1fs    ", slow_remain);
            attroff(A_BOLD);
        } else {
            mvprintw(y + 19, x, "Slow : --       ");
        }
        mvprintw(y + 20, x, "                ");
    } else {
        // 무적 아이템만 표시
        mvprintw(y + 18, x, "Cyan : Immune   ");
        if (immune_remain >= 0.0) {
            attron(A_BOLD);
            mvprintw(y + 19, x, "Immune : %.1fs  ", immune_remain);
            attroff(A_BOLD);
        } else {
            mvprintw(y + 19, x, "Immune : --     ");
        }
        mvprintw(y + 20, x, "                ");
    }

    // ── 전체 미션 달성 시 클리어 강조 표시
    mvprintw(y + 22, x, "                ");
    if (mission_len_ok && mission_growth_ok && mission_poison_ok && mission_gate_ok) {
        attron(A_BOLD);
        mvprintw(y + 22, x, "!! CLEAR !!");
        attroff(A_BOLD);
    }

    attroff(COLOR_PAIR(9));
}
