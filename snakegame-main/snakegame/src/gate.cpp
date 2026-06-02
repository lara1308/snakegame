// ────────────────────────────────────────────────
// gate.cpp
// 게이트 생성·재생성, 텔레포트 처리,
// 진출 방향 우선순위 로직을 담당하는 Gate 클래스 구현
// ────────────────────────────────────────────────

#include "gate.hpp"
#include <cstdlib>
#include <ctime>
#include <vector>
#include <utility>

// 방향 상수 (game.hpp의 Direction enum과 동일)
static const int G_UP    = 0;
static const int G_DOWN  = 1;
static const int G_LEFT  = 2;
static const int G_RIGHT = 3;

// 방향별 이동 오프셋 (UP, DOWN, LEFT, RIGHT)
static const int GATE_DX[4] = {-1,  1,  0,  0};
static const int GATE_DY[4] = { 0,  0, -1,  1};

// ── 내부 유틸 ────────────────────────────────────────────────────────────────

// 가장자리 벽의 경우 방향 반환: 상단→DOWN, 하단→UP, 좌측→RIGHT, 우측→LEFT
// 내부 벽이면 -1 반환
int Gate::getEdgeDir(int x, int y, int rows, int cols) const {
    if (x == 0)        return G_DOWN;
    if (x == rows - 1) return G_UP;
    if (y == 0)        return G_RIGHT;
    if (y == cols - 1) return G_LEFT;
    return -1; // 내부 벽
}

// ── 게이트 생성 ──────────────────────────────────────────────────────────────

void Gate::spawn(std::vector<std::vector<int>>& map) {
    const int rows = static_cast<int>(map.size());
    const int cols = static_cast<int>(map[0].size());

    // 일반 벽(1) 위치만 수집
    std::vector<std::pair<int,int>> wall_positions;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (map[i][j] == 1) {
                wall_positions.push_back({i, j});
            }
        }
    }

    if (wall_positions.size() < 2) return;

    // 겹치지 않는 두 위치 랜덤 선택
    int idx1 = std::rand() % static_cast<int>(wall_positions.size());
    int idx2;
    do {
        idx2 = std::rand() % static_cast<int>(wall_positions.size());
    } while (idx2 == idx1);

    gate1 = wall_positions[idx1];
    gate2 = wall_positions[idx2];
    exists = true;

    // 맵에 게이트 표시 (7)
    map[gate1.first][gate1.second] = 7;
    map[gate2.first][gate2.second] = 7;
}

// 기존 게이트를 벽(1)으로 복원 후 새 위치에 재생성
void Gate::respawn(std::vector<std::vector<int>>& map) {
    // 기존 게이트 제거 (7 → 1로 복원)
    if (exists) {
        map[gate1.first][gate1.second] = 1;
        map[gate2.first][gate2.second] = 1;
        exists = false;
    }
    // 새 위치에 생성
    spawn(map);
}

// ── 게이트 여부 확인 ─────────────────────────────────────────────────────────

bool Gate::is_gate(int x, int y) const {
    if (!exists) return false;
    return (gate1.first == x && gate1.second == y) ||
           (gate2.first == x && gate2.second == y);
}

// ── 텔레포트: 출구 좌표 + 새 방향 반환 ────────────────────────────────────────

std::pair<std::pair<int,int>, int> Gate::teleport_with_dir(
    int in_x, int in_y, int in_dir,
    const std::vector<std::vector<int>>& map) const
{
    // 출구 게이트 결정
    const int rows = static_cast<int>(map.size());
    const int cols = static_cast<int>(map[0].size());

    int gx, gy;
    if (gate1.first == in_x && gate1.second == in_y) {
        gx = gate2.first;
        gy = gate2.second;
    } else {
        gx = gate1.first;
        gy = gate1.second;
    }

    // 가장자리 벽이면 고정 방향 우선
    const int edge_dir = getEdgeDir(gx, gy, rows, cols);

    // 우선순위: ① 진입방향, ② 시계방향, ③ 역시계방향, ④ 반대방향
    // 가장자리 벽은 항상 안쪽(edge_dir)으로 나오도록 우선순위 보정
    int try_dirs[4];
    if (edge_dir != -1) {
        // 가장자리: edge_dir 방향이 막혀있지 않으면 그쪽으로 진출
        try_dirs[0] = edge_dir;
        try_dirs[1] = (edge_dir + 1) % 4;
        try_dirs[2] = (edge_dir + 3) % 4;
        try_dirs[3] = (edge_dir + 2) % 4;
    } else {
        // 내부 벽: 진입 방향 기준 우선순위
        try_dirs[0] = in_dir;
        try_dirs[1] = (in_dir + 1) % 4; // 시계방향
        try_dirs[2] = (in_dir + 3) % 4; // 역시계방향
        try_dirs[3] = (in_dir + 2) % 4; // 반대방향
    }

    // 진출 가능한 방향 탐색 (벽/면역벽/게이트/뱀 제외)
    for (int k = 0; k < 4; ++k) {
        const int dir = try_dirs[k];
        const int nx  = gx + GATE_DX[dir];
        const int ny  = gy + GATE_DY[dir];
        if (nx < 0 || nx >= rows || ny < 0 || ny >= cols) continue;
        const int cell = map[nx][ny];
        // 이동 불가 셀 제외
        if (cell == 1 || cell == 2 || cell == 7) continue;
        return {{nx, ny}, dir};
    }

    // 모든 방향이 막혀있으면 게이트 위치 그대로 반환 (게임오버 처리)
    return {{gx, gy}, in_dir};
}
