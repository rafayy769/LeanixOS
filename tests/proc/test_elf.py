import pytest
pytestmark = pytest.mark.elf

def assert_passed(result):
    """Helper to check if test passed."""
    if "PASSED:" in result:
        return True
    if "FAILED:" in result:
        raise AssertionError(f"{result}")
    raise AssertionError(f"{result}")


def test_elf_check_valid_header(runner):
    """Test ELF header validation with a valid ELF file."""
    result = runner.send_serial("test_elf_check_valid_header")
    assert_passed(result)


def test_elf_load_program(runner):
    """Test loading a complete ELF program."""
    result = runner.send_serial("test_elf_load_program")
    assert_passed(result)


def test_elf_load_nonexistent(runner):
    """Test that loading a nonexistent file fails gracefully."""
    result = runner.send_serial("test_elf_load_nonexistent")
    assert_passed(result)

def test_elf_load_null_args(runner):
    """Test graceful failure when NULL arguments are passed."""
    result = runner.send_serial("test_elf_load_null_args")
    assert_passed(result)
    
def test_elf_check_header_content(runner):
    """Hidden: Verify ELF header validation catches corrupted headers."""
    result = runner.send_serial("test_elf_check_header_content")
    assert_passed(result)
    
def test_elf_bss_zeroing(runner):
    """Hidden: Verify loader correctly zeros memory when mem_size > file_size (.bss)."""
    result = runner.send_serial("test_elf_bss_zeroing")
    assert_passed(result)