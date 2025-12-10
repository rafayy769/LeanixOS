import pytest

pytestmark = pytest.mark.vga    # vga test suite


def test_vga_entry(runner):
    result = runner.send_serial("vga_entry")
    assert "PASSED*" == result

def test_vga_cursor(runner):
    result = runner.send_serial("vga_cursor")
    assert "PASSED*" == result

def test_vga_entry_overwrite(runner):
    result = runner.send_serial("vga_entry_overwrite")
    assert "PASSED*" == result

def test_vga_entry_colors(runner):
    result = runner.send_serial("vga_color")
    assert "PASSED*" == result

def test_vga_entry_boundaries(runner):
    result = runner.send_serial("vga_entry_boundaries")
    assert "PASSED*" == result

def test_vga_entry_hidden(runner):
    result = runner.send_serial("vga_entry_hidden")
    assert "PASSED*" == result

def test_vga_cursor_hidden(runner):
    result = runner.send_serial("vga_cursor_hidden")
    assert "PASSED*" == result