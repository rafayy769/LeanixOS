import pytest

pytestmark = pytest.mark.tty    # tty test suite

def test_terminal_getc(runner):
    runner.send_monitor(f"sendkey a")
    result = runner.send_serial("terminal_getc")
    assert result == "a*"

def test_terminal_read(runner):
	test_chars = "asd98123nv1qo"
	for ch in test_chars:
		runner.send_monitor(f"sendkey {ch}")
	runner.send_monitor("sendkey ret")
	result = runner.send_serial("terminal_read")
	assert f"{test_chars}*" == result

def test_terminal_cursor(runner):
    result = runner.send_serial("terminal_cursor")
    assert result == "PASSED*"

def test_terminal_clear(runner):
    result = runner.send_serial("terminal_clear")
    assert result == "PASSED*"

def test_terminal_column(runner):
    result = runner.send_serial("terminal_column")
    assert result == "PASSED*"

def test_terminal_putc(runner):
    result = runner.send_serial("terminal_putc")
    assert result == "PASSED*"

def test_terminal_write(runner):
    result = runner.send_serial("terminal_write")
    assert result == "PASSED*"

def test_terminal_column(runner):
    result = runner.send_serial("terminal_column")
    assert result == "PASSED*"
    
def test_terminal_scroll(runner):
    result = runner.send_serial("terminal_scroll")
    assert result == "PASSED*"

def test_terminal_colour(runner):
    result = runner.send_serial("terminal_colour")
    assert result == "PASSED*"

# DISABLED THESE FOR NOW, weird tests
# def test_terminal_text_colour(runner):
#     result = runner.send_serial("terminal_text_color")
#     assert result == "PASSED*"

# def test_terminal_bg_colour(runner):
#     result = runner.send_serial("terminal_bg_color")
#     assert result == "PASSED*"
    
def test_terminal_echo(runner):
	test_chars = "asd98123nv1qo"
	for ch in test_chars:
		runner.send_monitor(f"sendkey {ch}")
	runner.send_monitor("sendkey ret")
	result = runner.send_serial("terminal_echo")
	assert f"PASSED*" == result