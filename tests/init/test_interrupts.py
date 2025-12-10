
import pytest

pytestmark = pytest.mark.interrupts    # interrupts test suite

def test_intr_reg(runner):
	result = runner.send_serial("intr_reg")
	assert "PASSED*" == result

def test_intr_unreg(runner):
	result = runner.send_serial("intr_unreg")
	assert "PASSED*" == result

def test_intr_multi(runner):
	result = runner.send_serial("intr_multi")
	assert "PASSED*" == result
