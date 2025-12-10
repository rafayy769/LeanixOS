import pytest

pytestmark = pytest.mark.kheap

def assert_passed(result: str):
    """Helper: ensure PASSED and not FAILED."""
    assert "FAILED" not in result, f"Allocator failed: {result}"
    assert "PASSED" in result, f"Unexpected output: {result}"

def test_init(runner):
    assert "PASSED*" in runner.send_serial("kheap_init")


def test_alloc_small(runner):
    assert "PASSED*" in runner.send_serial("kheap_alloc_small")


def test_alloc_exact(runner):
    assert "PASSED*" in runner.send_serial("kheap_alloc_exact")


def test_split(runner):
    assert "PASSED*" in runner.send_serial("kheap_split")


def test_free_reuse(runner):
    assert "PASSED*" in runner.send_serial("kheap_free_reuse")


def test_coalesce(runner):
    assert "PASSED*" in runner.send_serial("kheap_coalesce")


def test_double_free(runner):
    assert "PASSED*" in runner.send_serial("kheap_double_free")


def test_invalid_free(runner):
    assert "PASSED*" in runner.send_serial("kheap_invalid_free")


def test_realloc_shrink(runner):
    assert "PASSED*" in runner.send_serial("kheap_realloc_shrink")


def test_realloc_expand(runner):
    assert "PASSED*" in runner.send_serial("kheap_realloc_expand")


def test_realloc_null(runner):
    assert "PASSED*" in runner.send_serial("kheap_realloc_null")


def test_realloc_zero(runner):
    assert "PASSED*" in runner.send_serial("kheap_realloc_zero")


def test_oom(runner):
    assert "PASSED*" in runner.send_serial("kheap_oom")


def test_stress_pattern(runner):
    assert "PASSED*" in runner.send_serial("kheap_stress_pattern")


# --- HIDDEN START HERE

def test_kheap_fragmentation_coalescing(runner):
    result = runner.send_serial("kheap_fragmentation_coalescing")
    assert_passed(result)

def test_kheap_alignment_check(runner):
    result = runner.send_serial("kheap_alignment_check")
    assert_passed(result)

def test_kheap_random_stress(runner):
    result = runner.send_serial("kheap_random_stress")
    assert_passed(result)

def test_kheap_realloc_integrity(runner):
    result = runner.send_serial("kheap_realloc_integrity")
    assert_passed(result)

def test_kheap_buddy_symmetry(runner):
    result = runner.send_serial("kheap_buddy_symmetry")
    assert_passed(result)

# def test_kheap_buddy_multilevel(runner):
#     result = runner.send_serial("kheap_buddy_multilevel")
#     assert_passed(result)
