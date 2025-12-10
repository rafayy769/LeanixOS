import pytest

pytestmark = pytest.mark.shell    # shell test suite


def test_shell_echo(runner):
    result = runner.send_serial("shell_echo")
    assert "PASSED*" == result

def test_shell_repeat(runner):
    result = runner.send_serial("shell_repeat")
    assert "PASSED*" == result

# def test_shell_clear(runner):
#     result =  runner.send_serial("shell_clear")
#     assert "PASSED*" == result

def  test_shell_colour(runner):
    result = runner.send_serial("shell_colour")
    assert "PASSED*" == result

def test_shell_bgcolour(runner):
    result = runner.send_serial("shell_bgcolour")
    assert "PASSED*" == result
