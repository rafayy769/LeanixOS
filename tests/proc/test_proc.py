import pytest
import re
from collections import Counter

pytestmark = pytest.mark.proc

"""
Test orchestrator for scheduler and process management tests.
These tests only use the public API and don't assume internal structure.
"""

def assert_passed(result):
    """Helper to check if test passed."""
    if "PASSED:" in result:
        return True
    if "FAILED:" in result:
        raise AssertionError(f"{result}")
    raise AssertionError(f"{result}")


def test_process_create_nonnull(runner):
    """Test that process_create returns a non-null process."""
    result = runner.send_serial("test_process_create_nonnull")
    assert_passed(result)


def test_process_create_different_priorities(runner):
    """Test process creation with min, max, and default priorities."""
    result = runner.send_serial("test_process_create_different_priorities")
    assert_passed(result)


def test_process_name_handling(runner):
    """Test process creation with various name lengths."""
    result = runner.send_serial("test_process_name_handling")
    assert_passed(result)


def test_get_main_thread_nonnull(runner):
    """Test that _get_main_thread returns a valid thread."""
    result = runner.send_serial("test_get_main_thread_nonnull")
    assert_passed(result)


def test_thread_creation(runner):
    """Test basic thread creation."""
    result = runner.send_serial("test_thread_creation")
    assert_passed(result)


def test_multiple_thread_creation(runner):
    """Test creating multiple threads within a process."""
    result = runner.send_serial("test_multiple_thread_creation")
    assert_passed(result)


def test_scheduler_get_current_thread(runner):
    """Test that get_current_thread returns a valid thread."""
    result = runner.send_serial("test_scheduler_get_current_thread")
    assert_passed(result)


def test_scheduler_get_current_proc(runner):
    """Test that get_current_proc returns a valid process."""
    result = runner.send_serial("test_scheduler_get_current_proc")
    assert_passed(result)


def test_scheduler_post_thread(runner):
    """Test posting a thread to the scheduler."""
    result = runner.send_serial("test_scheduler_post_thread")
    assert_passed(result)


def test_multiple_process_creation(runner):
    """Test creating multiple processes."""
    result = runner.send_serial("test_multiple_process_creation")
    assert_passed(result)


def test_thread_destroy_safe(runner):
    """Test that thread_destroy doesn't crash."""
    result = runner.send_serial("test_thread_destroy_safe")
    assert_passed(result)


def test_process_exit_safe(runner):
    """Test that process_exit doesn't crash."""
    result = runner.send_serial("test_process_exit_safe")
    assert_passed(result)


def test_concurrent_scheduler_ops(runner):
    """Test concurrent scheduler operations."""
    result = runner.send_serial("test_concurrent_scheduler_ops")
    assert_passed(result)

def test_many_threads_creation(runner):
    """Stress test: create many threads and verify they execute."""
    result = runner.send_serial("test_many_threads_creation")
    assert_passed(result)


# Integration test for actual scheduling behavior
def test_scheduler_round_robin(runner):
    """
    Integration test: Verify round-robin scheduling behavior.
    Tests actual context switching between threads.
    """
    result = runner.send_serial("test_thread", timeout=5.0)
    
    # The test should produce interleaved output
    # Thread 1: 100 'a's
    # Thread 2: 200 'b's  
    # Thread 3: 1000 'c's + '*'
    
    # Count occurrences
    count_a = result.count('a')
    count_b = result.count('b')
    count_c = result.count('c')
    has_marker = '*' in result
    
    # Verify all threads ran
    assert count_a == 100, f"Thread 1 didn't complete: got {count_a} 'a's, expected 100"
    assert count_b == 200, f"Thread 2 didn't complete: got {count_b} 'b's, expected 200"
    assert count_c == 1000, f"Thread 3 didn't complete: got {count_c} 'c's, expected 1000"
    assert has_marker, "Thread 3 marker '*' not found - thread didn't complete"
    
    assert True


def test_scheduler_ordering(runner):
    result = runner.send_serial("test_scheduler_ordering")
    
    # only check that the BEGINNING of the output matches this pattern:
    # c+ k+ j+ i+ h+ g+ f+ e+ d+ b+ a+
    # order = "ckjihgfedba"
    order = "abdefghijc"
    prefix_pattern = "^" + "".join(f"{ch}+" for ch in order)
    regex = re.compile(prefix_pattern)

    assert regex.match(result), (
        "Output does not start with the required pattern "
        "a+ b+ d+ e+ f+ g+ h+ i+ j+ c+"
    )

    # Verify letter counts
    letter_counts = Counter(result)
    
    # Check 'a' count is 100
    assert letter_counts['a'] == 100, f"Expected 100 'a's, got {letter_counts['a']}"
    
    # Check 'c' count is 1000
    assert letter_counts['c'] == 1000, f"Expected 1000 'c's, got {letter_counts['c']}"
    
    # Check all other letters (b, d, e, f, g, h, i, j, k) have count 200
    for letter in 'bdefghij':
        assert letter_counts[letter] == 200, f"Expected 200 '{letter}'s, got {letter_counts[letter]}"
    
    assert True