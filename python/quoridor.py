# quoridor.py
from __future__ import annotations

from dataclasses import dataclass
from enum import Enum, auto
from typing import Iterable, Iterator, Optional, Union

from collections import deque

BOARD_SIZE = 9  # 9x9 cells; walls are placeable on an 8x8 grid


class Orientation(Enum):
    H = auto()
    V = auto()


class Player(Enum):
    WHITE = auto()
    BLACK = auto()

    def opponent(self) -> "Player":
        return Player.BLACK if self is Player.WHITE else Player.WHITE


@dataclass(frozen=True)
class PawnMove:
    to: tuple[int, int]


@dataclass(frozen=True)
class WallMove:
    r: int  # top-left of the wall's 2-segment placement area, 0..7
    c: int  # 0..7
    o: Orientation


Move = Union[PawnMove, WallMove]


@dataclass(frozen=True)
class State:
    white: tuple[int, int] = (8, 4)
    black: tuple[int, int] = (0, 4)
    # Store wall segments that block adjacency between cells:
    # - horizontal_walls: set of (r,c) segments that block (r,c) <-> (r+1,c)
    # - vertical_walls:   set of (r,c) segments that block (r,c) <-> (r,c+1)
    horizontal_walls: frozenset[tuple[int, int]] = frozenset()
    vertical_walls: frozenset[tuple[int, int]] = frozenset()
    to_move: Player = Player.WHITE
    white_walls: int = 10
    black_walls: int = 10

    def coord(self, p: Player) -> tuple[int, int]:
        return self.white if p is Player.WHITE else self.black

    def with_move(self, move: Move) -> "State":
        if isinstance(move, PawnMove):
            if self.to_move is Player.WHITE:
                return State(
                    white=move.to,
                    black=self.black,
                    horizontal_walls=self.horizontal_walls,
                    vertical_walls=self.vertical_walls,
                    to_move=self.to_move.opponent(),
                    white_walls=self.white_walls,
                    black_walls=self.black_walls,
                )
            else:
                return State(
                    white=self.white,
                    black=move.to,
                    horizontal_walls=self.horizontal_walls,
                    vertical_walls=self.vertical_walls,
                    to_move=self.to_move.opponent(),
                    white_walls=self.white_walls,
                    black_walls=self.black_walls,
                )
        else:
            # Place a wall and decrement the wall count for the mover
            hw = set(self.horizontal_walls)
            vw = set(self.vertical_walls)
            place_wall_segments_inplace(hw, vw, move)

            if self.to_move is Player.WHITE:
                return State(
                    white=self.white,
                    black=self.black,
                    horizontal_walls=frozenset(hw),
                    vertical_walls=frozenset(vw),
                    to_move=self.to_move.opponent(),
                    white_walls=self.white_walls - 1,
                    black_walls=self.black_walls,
                )
            else:
                return State(
                    white=self.white,
                    black=self.black,
                    horizontal_walls=frozenset(hw),
                    vertical_walls=frozenset(vw),
                    to_move=self.to_move.opponent(),
                    white_walls=self.white_walls,
                    black_walls=self.black_walls - 1,
                )

    def walls_left(self, p: Player) -> int:
        return self.white_walls if p is Player.WHITE else self.black_walls


# ---------- Coordinate / adjacency helpers ----------


def in_bounds(r: int, c: int) -> bool:
    return 0 <= r < BOARD_SIZE and 0 <= c < BOARD_SIZE


def adjacent(a: tuple[int, int], b: tuple[int, int]) -> bool:
    r1, c1 = a
    r2, c2 = b
    return abs(r1 - r2) + abs(c1 - c2) == 1


def wall_blocks_between(s: State, a: tuple[int, int], b: tuple[int, int]) -> bool:
    # a and b must be cardinal-adjacent
    r1, c1 = a
    r2, c2 = b
    if r1 == r2:
        # horizontal move -> check vertical wall between columns
        c = min(c1, c2)
        return (r1, c) in s.vertical_walls
    if c1 == c2:
        # vertical move -> check horizontal wall between rows
        r = min(r1, r2)
        return (r, c1) in s.horizontal_walls
    return True  # non-cardinal treated as blocked


# ---------- Wall placement rules ----------


def is_valid_wall_placement(s: State, mv: WallMove) -> bool:
    r, c, o = mv.r, mv.c, mv.o
    if not (0 <= r <= BOARD_SIZE - 2 and 0 <= c <= BOARD_SIZE - 2):
        return False

    # Check overlap and crossing using the segment representation.
    if o is Orientation.H:
        # H wall at (r,c) occupies horizontal segments (r,c) and (r,c+1)
        # Overlap if either already present
        if (r, c) in s.horizontal_walls or (r, c + 1) in s.horizontal_walls:
            return False
        # Crossing if a vertical wall spans exactly across (r,c) boundary:
        # That means a vertical wall with segments at (r, c) and (r + 1, c)
        if (r, c) in s.vertical_walls and (r + 1, c) in s.vertical_walls:
            return False
    else:
        # V wall at (r,c) occupies vertical segments (r,c) and (r+1,c)
        if (r, c) in s.vertical_walls or (r + 1, c) in s.vertical_walls:
            return False
        # Crossing if a horizontal wall spans across (r,c):
        if (r, c) in s.horizontal_walls and (r, c + 1) in s.horizontal_walls:
            return False

    # Temporarily place and ensure both players still have a path
    hw = set(s.horizontal_walls)
    vw = set(s.vertical_walls)
    place_wall_segments_inplace(hw, vw, mv)

    if not _has_path_to_goal(
        State(
            white=s.white,
            black=s.black,
            horizontal_walls=frozenset(hw),
            vertical_walls=frozenset(vw),
            to_move=s.to_move,
            white_walls=s.white_walls,
            black_walls=s.black_walls,
        ),
        Player.WHITE,
    ):
        return False

    if not _has_path_to_goal(
        State(
            white=s.white,
            black=s.black,
            horizontal_walls=frozenset(hw),
            vertical_walls=frozenset(vw),
            to_move=s.to_move,
            white_walls=s.white_walls,
            black_walls=s.black_walls,
        ),
        Player.BLACK,
    ):
        return False

    return True


def place_wall_segments_inplace(hw: set[tuple[int, int]], vw: set[tuple[int, int]], mv: WallMove) -> None:
    r, c, o = mv.r, mv.c, mv.o
    if o is Orientation.H:
        hw.add((r, c))
        hw.add((r, c + 1))
    else:
        vw.add((r, c))
        vw.add((r + 1, c))


# ---------- Pawn movement (Quoridor rules) ----------

# Direction vectors for cardinal moves: N, S, E, W
CARDINALS: list[tuple[int, int]] = [(-1, 0), (1, 0), (0, 1), (0, -1)]


def _dir_sides(dr: int, dc: int) -> tuple[tuple[int, int], tuple[int, int]]:
    # Left/right relative to (dr,dc): rotate (dr,dc) by ±90°
    # left = (-dc, dr), right = (dc, -dr)
    return (-dc, dr), (dc, -dr)


def legal_pawn_moves(s: State, p: Player) -> list[PawnMove]:
    me = s.coord(p)
    opp = s.coord(p.opponent())
    r, c = me
    moves: list[PawnMove] = []

    for dr, dc in CARDINALS:
        nr, nc = r + dr, c + dc
        if not in_bounds(nr, nc):
            continue
        if wall_blocks_between(s, (r, c), (nr, nc)):
            continue

        if (nr, nc) != opp:
            # Free adjacent square
            moves.append(PawnMove((nr, nc)))
        else:
            # Opponent adjacent: try straight jump if possible
            jr, jc = nr + dr, nc + dc
            if in_bounds(jr, jc) and not wall_blocks_between(s, (nr, nc), (jr, jc)):
                moves.append(PawnMove((jr, jc)))
            else:
                # Can't jump straight (edge or wall behind opponent) -> diagonal
                left, right = _dir_sides(dr, dc)
                for sdr, sdc in (left, right):
                    drr, dcc = nr + sdr, nc + sdc
                    if not in_bounds(drr, dcc):
                        continue
                    # Must be able to go me -> opp and opp -> diagonal
                    if wall_blocks_between(s, (r, c), (nr, nc)):
                        continue
                    if wall_blocks_between(s, (nr, nc), (drr, dcc)):
                        continue
                    moves.append(PawnMove((drr, dcc)))

    return moves


# ---------- Path existence and distance ----------


def _neighbors_ignoring_pawns(s: State, node: tuple[int, int]) -> Iterator[tuple[int, int]]:
    r, c = node
    for dr, dc in CARDINALS:
        nr, nc = r + dr, c + dc
        if not in_bounds(nr, nc):
            continue
        if not wall_blocks_between(s, (r, c), (nr, nc)):
            yield (nr, nc)


def _has_path_to_goal(s: State, p: Player) -> bool:
    # Rule check: walls must not block any player's path to goal row.
    start = s.coord(p)
    goal_row = 0 if p is Player.WHITE else BOARD_SIZE - 1

    q: deque[tuple[int, int]] = deque([start])
    seen = {start}
    while q:
        r, c = q.popleft()
        if r == goal_row:
            return True
        for nb in _neighbors_ignoring_pawns(s, (r, c)):
            if nb not in seen:
                seen.add(nb)
                q.append(nb)
    return False


def distance_to_goal(s: State, p: Player) -> int:
    # Shortest path length to goal row ignoring pawns (standard heuristic)
    start = s.coord(p)
    goal_row = 0 if p is Player.WHITE else BOARD_SIZE - 1

    q: deque[tuple[tuple[int, int], int]] = deque([(start, 0)])
    seen = {start}
    while q:
        (r, c), d = q.popleft()
        if r == goal_row:
            return d
        for nb in _neighbors_ignoring_pawns(s, (r, c)):
            if nb not in seen:
                seen.add(nb)
                q.append((nb, d + 1))
    return 10**9  # unreachable shouldn't happen with valid walls


# ---------- Legal move generation ----------


def legal_moves(s: State) -> list[Move]:
    moves: list[Move] = []

    # Pawn moves
    moves.extend(legal_pawn_moves(s, s.to_move))

    # Wall moves (if walls remaining)
    if s.walls_left(s.to_move) > 0:
        for r in range(BOARD_SIZE - 1):
            for c in range(BOARD_SIZE - 1):
                # H
                wm = WallMove(r, c, Orientation.H)
                if is_valid_wall_placement(s, wm):
                    moves.append(wm)
                # V
                wm = WallMove(r, c, Orientation.V)
                if is_valid_wall_placement(s, wm):
                    moves.append(wm)

    return moves


# ---------- Terminal, evaluation, and a simple alpha-beta ----------


def is_terminal(s: State) -> Optional[Player]:
    if s.white[0] == 0:
        return Player.WHITE
    if s.black[0] == BOARD_SIZE - 1:
        return Player.BLACK
    return None


def evaluate(s: State, maximizing_for: Player) -> int:
    # Simple and strong baseline:
    # - Distance to goal (lower is better)
    # - Walls remaining (more is better)
    d_w = distance_to_goal(s, Player.WHITE)
    d_b = distance_to_goal(s, Player.BLACK)
    walls_term = (s.white_walls - s.black_walls) * 2

    score = (d_b - d_w) + walls_term  # White wants this high; Black wants low
    return score if maximizing_for is Player.WHITE else -score


def alpha_beta(
    s: State,
    depth: int,
    alpha: int,
    beta: int,
    maximizing_for: Player,
) -> tuple[int, Optional[Move]]:
    winner = is_terminal(s)
    if winner is not None:
        if winner is maximizing_for:
            return 10_000, None
        else:
            return -10_000, None

    if depth == 0:
        return evaluate(s, maximizing_for), None

    best_move: Optional[Move] = None
    if s.to_move is maximizing_for:
        value = -(10**9)
        for mv in legal_moves(s):
            nxt = s.with_move(mv)
            v, _ = alpha_beta(nxt, depth - 1, alpha, beta, maximizing_for)
            if v > value:
                value, best_move = v, mv
            alpha = max(alpha, value)
            if alpha >= beta:
                break
        return value, best_move
    else:
        value = 10**9
        for mv in legal_moves(s):
            nxt = s.with_move(mv)
            v, _ = alpha_beta(nxt, depth - 1, alpha, beta, maximizing_for)
            if v < value:
                value, best_move = v, mv
            beta = min(beta, value)
            if alpha >= beta:
                break
        return value, best_move


# ---------- Example usage ----------

if __name__ == "__main__":
    s = State()
    print("Initial:", s)
    print("Legal moves at start:", len(legal_moves(s)))
    score, mv = alpha_beta(s, depth=3, alpha=-(10**9), beta=10**9, maximizing_for=s.to_move)
    print("Depth-2 best:", score, mv)
