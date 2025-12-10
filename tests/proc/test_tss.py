import pytest
pytestmark = pytest.mark.tss

def assert_passed(result):
    """Helper to check if test passed."""
    if "PASSED:" in result:
        return True
    if "FAILED:" in result:
        raise AssertionError(f"{result}")
    raise AssertionError(f"{result}")


def test_tss_global_access(runner):
    """Test that tss_get_global() returns a valid, consistent TSS pointer."""
    result = runner.send_serial("test_tss_global_access")
    assert_passed(result)


def test_tss_esp0_update(runner):
    """Test that tss_update_esp0() correctly updates the TSS esp0 field."""
    result = runner.send_serial("test_tss_esp0_update")
    assert_passed(result)

def test_tss_layout_and_init(runner):
    """Hidden: Verify TSS structure layout and initial field values."""
    result = runner.send_serial("test_tss_layout_and_init")
    assert_passed(result)