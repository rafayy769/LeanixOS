import pytest

pytestmark = pytest.mark.sys    # syscall test suite


def test_syscall_register(runner):
    result = runner.send_serial("syscall_register")
    assert "PASSED*" == result

def test_syscall_read(runner):
    test_chars = "asd98123nv1qo"
    for ch in test_chars:
        runner.send_monitor(f"sendkey {ch}")
    runner.send_monitor("sendkey ret")
    result = runner.send_serial("syscall_read")
    assert f"{test_chars}*" == result

def test_syscall_write(runner):
    result = runner.send_serial("syscall_write")
    assert "PASSED*" == result

def test_syscall_read_enforced(runner):
    test_chars = "enforce123"
    for ch in test_chars:
        runner.send_monitor(f"sendkey {ch}")
    runner.send_monitor("sendkey ret")
    result = runner.send_serial("syscall_read_enforced")
    assert "PASSED*" == result

def test_syscall_write_enforced(runner):
    result = runner.send_serial("syscall_write_enforced")
    assert "PASSED*" == result