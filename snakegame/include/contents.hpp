#pragma once
#include <string>

class Contents {
public:
    // 스코어보드 상태 갱신
    void set_status(int cur_len, int max_len, int growth, int poison, int gate);

    // 미션 목표 및 달성 여부 갱신
    void set_mission(int m_len, int m_growth, int m_poison, int m_gate,
                     bool len_ok, bool growth_ok, bool poison_ok, bool gate_ok);

    // 현재 스테이지 번호 갱신 (1~4)
    void set_stage(int stage);

    // 아이템 남은 시간 갱신 (초 단위, -1이면 비활성)
    void set_item_timer(double immune_remain, double slow_remain);

    // 스코어보드 전체 출력
    void draw(int x, int y) const;

private:
    // Score Board
    int cur_length  = 0;
    int max_length  = 0;
    int growth_cnt  = 0;
    int poison_cnt  = 0;
    int gate_cnt    = 0;

    // Mission
    int  mission_len     = 0;
    int  mission_growth  = 0;
    int  mission_poison  = 0;
    int  mission_gate    = 0;
    bool mission_len_ok     = false;
    bool mission_growth_ok  = false;
    bool mission_poison_ok  = false;
    bool mission_gate_ok    = false;

    // 현재 스테이지
    int current_stage = 1;

    // 특수 아이템 남은 시간 (-1: 비활성)
    double immune_remain = -1.0;
    double slow_remain   = -1.0;
};
