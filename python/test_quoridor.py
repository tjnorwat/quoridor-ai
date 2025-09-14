import pytest

from quoridor import (
    BOARD_SIZE,
    Orientation,
    Player,
    State,
    PawnMove,
    WallMove,
    legal_moves,
    legal_pawn_moves,
    is_valid_wall_placement,
    distance_to_goal,
    is_terminal,
    evaluate,
    wall_blocks_between,
)


def targets_of_pawn_moves(moves):
    return {mv.to for mv in moves if isinstance(mv, PawnMove)}


def only_pawn_moves(moves):
    return [m for m in moves if isinstance(m, PawnMove)]


def only_wall_moves(moves):
    return [m for m in moves if isinstance(m, WallMove)]


def make_h_segments_at(r, c):
    # H wall at (r,c) occupies segments (r,c) and (r,c+1)
    return {(r, c), (r, c + 1)}


def make_v_segments_at(r, c):
    # V wall at (r,c) occupies segments (r,c) and (r+1,c)
    return {(r, c), (r + 1, c)}


def test_initial_legal_moves_counts_and_targets():
    s = State()
    mvs = legal_moves(s)
    pawn = only_pawn_moves(mvs)
    walls = only_wall_moves(mvs)

    # On an empty board from (8,4), White can go N, E, W (not S).
    assert targets_of_pawn_moves(pawn) == {(7, 4), (8, 3), (8, 5)}

    # All 8x8 placements for both orientations are legal on an empty board.
    assert len(walls) == (BOARD_SIZE - 1) ** 2 * 2  # 64 * 2 = 128
    assert len(pawn) == 3
    assert len(mvs) == 131


def test_wall_blocks_between_vertical_and_horizontal():
    # Vertical segment blocks horizontal adjacency at that row/column
    s1 = State(vertical_walls=frozenset({(5, 4)}))
    assert wall_blocks_between(s1, (5, 4), (5, 5)) is True
    assert wall_blocks_between(s1, (5, 5), (5, 4)) is True
    # Does not block vertical adjacency
    assert wall_blocks_between(s1, (4, 4), (5, 4)) is False

    # Both segments of a vertical wall block two adjacent rows
    s2 = State(vertical_walls=frozenset({(5, 4), (6, 4)}))
    assert wall_blocks_between(s2, (6, 4), (6, 5)) is True

    # Horizontal segment blocks vertical adjacency at that row/column
    s3 = State(horizontal_walls=frozenset({(4, 3)}))
    assert wall_blocks_between(s3, (4, 3), (5, 3)) is True
    assert wall_blocks_between(s3, (5, 3), (4, 3)) is True
    # Does not block horizontal adjacency
    assert wall_blocks_between(s3, (4, 3), (4, 4)) is False


def test_simple_pawn_moves_respect_walls():
    # Block White's east move from (8,4) -> (8,5) with a V wall at top-left (7,4)
    v_segments = make_v_segments_at(7, 4)  # {(7,4),(8,4)}
    s = State(vertical_walls=frozenset(v_segments))  # realistic placement
    moves = legal_pawn_moves(s, Player.WHITE)
    targets = targets_of_pawn_moves(moves)
    assert (8, 5) not in targets  # east blocked
    assert (7, 4) in targets  # north open
    assert (8, 3) in targets  # west open


def test_jump_straight_when_opponent_ahead_and_no_wall_behind():
    # White at (8,4), Black at (7,4), no walls -> straight jump available
    s = State(white=(8, 4), black=(7, 4))
    moves = legal_pawn_moves(s, Player.WHITE)
    targets = targets_of_pawn_moves(moves)

    # Allowed: straight jump to (6,4) and side moves from original square
    assert (6, 4) in targets
    assert (8, 3) in targets
    assert (8, 5) in targets
    # Diagonals should not be present when straight jump is available
    assert (7, 3) not in targets
    assert (7, 5) not in targets


def test_diagonal_when_wall_blocks_straight_jump():
    # White at (8,4), Black at (7,4). Block (7,4)->(6,4) with horizontal segment (6,4)
    s = State(
        white=(8, 4),
        black=(7, 4),
        horizontal_walls=frozenset({(6, 4)}),  # part of H wall at (6,4)
    )
    moves = legal_pawn_moves(s, Player.WHITE)
    targets = targets_of_pawn_moves(moves)

    # Straight jump is blocked
    assert (6, 4) not in targets
    # Diagonals around opponent are allowed
    assert (7, 3) in targets
    assert (7, 5) in targets
    # Side moves from original square are still allowed
    assert (8, 3) in targets
    assert (8, 5) in targets


def test_diagonal_partial_blocking_from_opponent_square():
    # Same as above, but block opponent->east to disallow NE diagonal
    s = State(
        white=(8, 4),
        black=(7, 4),
        horizontal_walls=frozenset({(6, 4)}),  # block straight jump
        vertical_walls=frozenset({(7, 4)}),  # block (7,4)->(7,5)
    )
    moves = legal_pawn_moves(s, Player.WHITE)
    targets = targets_of_pawn_moves(moves)

    assert (6, 4) not in targets
    assert (7, 5) not in targets  # blocked via opp->east
    assert (7, 3) in targets  # still open via opp->west
    assert (8, 3) in targets and (8, 5) in targets


def test_wall_overlap_and_crossing_invalid():
    s = State()

    # Place an H wall at (4,4)
    w1 = WallMove(4, 4, Orientation.H)
    assert is_valid_wall_placement(s, w1)
    s1 = s.with_move(w1)

    # Overlap another H wall at (4,5) (shares segment (4,5)) -> invalid
    w_overlap = WallMove(4, 5, Orientation.H)
    assert not is_valid_wall_placement(s1, w_overlap)

    # Crossing V wall at (4,4) through the middle -> invalid
    w_cross = WallMove(4, 4, Orientation.V)
    assert not is_valid_wall_placement(s1, w_cross)

    # A separate non-overlapping, non-crossing wall should still be valid
    w_ok = WallMove(2, 2, Orientation.V)
    assert is_valid_wall_placement(s1, w_ok)


def test_final_wall_that_blocks_all_paths_is_invalid():
    # Build a near-complete horizontal barrier between rows 4 and 5,
    # leaving only the far-right segment (4,8) open
    segs = set()
    for c in range(0, BOARD_SIZE - 2):  # 0..6
        segs |= make_h_segments_at(4, c)  # add (4,c) and (4,c+1)
    # This leaves segment (4,8) missing, so paths still exist
    s = State(horizontal_walls=frozenset(segs))

    # Placing the final H wall at (4,7) would add segments (4,7) and (4,8)
    # and fully sever any path from top to bottom -> invalid
    final_wall = WallMove(4, 7, Orientation.H)
    assert not is_valid_wall_placement(s, final_wall)


def test_walls_left_gates_wall_generation():
    s = State(white_walls=0, to_move=Player.WHITE)
    mvs = legal_moves(s)
    assert len(only_wall_moves(mvs)) == 0
    assert len(only_pawn_moves(mvs)) > 0


def test_with_move_toggles_and_counts_and_pawn_update():
    s = State()
    w = WallMove(4, 4, Orientation.H)
    assert is_valid_wall_placement(s, w)
    s1 = s.with_move(w)
    assert s1.to_move == Player.BLACK
    assert s1.white_walls == 9 and s1.black_walls == 10

    # Black pawn move: from (0,4) to (1,4)
    pm = PawnMove((1, 4))
    # Ensure it's legal in this state
    assert (1, 4) in targets_of_pawn_moves(legal_pawn_moves(s1, Player.BLACK))
    s2 = s1.with_move(pm)
    assert s2.black == (1, 4)
    assert s2.to_move == Player.WHITE


def test_is_terminal_and_distance_and_evaluate():
    s0 = State()
    # Distance to goal ignoring pawns on empty board is 8 for both
    assert distance_to_goal(s0, Player.WHITE) == 8
    assert distance_to_goal(s0, Player.BLACK) == 8

    # Symmetric start evaluates to 0 for both perspectives
    assert evaluate(s0, Player.WHITE) == 0
    assert evaluate(s0, Player.BLACK) == 0

    # Terminal states
    s_won = State(white=(0, 4))
    s_lost = State(black=(8, 4))
    assert is_terminal(s_won) == Player.WHITE
    assert is_terminal(s_lost) == Player.BLACK
