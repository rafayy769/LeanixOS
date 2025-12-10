
import pytest

pytestmark = pytest.mark.kbd    # kbd test suite

def test_kbd_basic(runner):

	# trigger a keypress
	runner.send_monitor("sendkey a")

	result = runner.send_serial("kbd_basic")
	assert "a*" == result

def test_kbd_multi(runner):
	
	test_chars = "asd98123nv1qo"
	for ch in test_chars:
		runner.send_monitor(f"sendkey {ch}")
	runner.send_monitor("sendkey ret")

	result = runner.send_serial("kbd_multi")
	assert f"{test_chars}*" == result

def test_kbd_capslock(runner):

	runner.send_monitor("sendkey caps_lock")		# enable caps lock
	runner.send_monitor("sendkey a")
	result = runner.send_serial("kbd_capslock")
	runner.send_monitor("sendkey caps_lock")		# disable caps lock

	assert "A*" == result


def test_kbd_shift(runner):

	# enable shift
	runner.send_monitor("sendkey shift-a")
	result = runner.send_serial("kbd_shift")

	assert "A*" == result
