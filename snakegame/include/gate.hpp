#pragma once
#include <vector>
#include <utility>

class Gate {
public:
    // 맵의 일반 벽(1) 중 랜덤 두 곳에 게이트 생성
    void spawn(std::vector<std::vector<int>>& map);

    // 기존 게이트를 맵에서 제거하고 새로운 위치에 재생성
    void respawn(std::vector<std::vector<int>>& map);

    std::pair<int,int> get_gate1() const { return gate1; }
    std::pair<int,int> get_gate2() const { return gate2; }

    bool is_gate(int x, int y) const;
    bool has_gate() const { return exists; }

    // 게이트 진입 시 출구 좌표 + 새 방향 반환
    std::pair<std::pair<int,int>, int> teleport_with_dir(
        int in_x, int in_y, int in_dir,
        const std::vector<std::vector<int>>& map) const;

private:
    std::pair<int,int> gate1, gate2;
    bool exists = false;

    // 벽 위치에서 가장자리 여부 및 방향 판별
    int getEdgeDir(int x, int y, int rows, int cols) const;
};
