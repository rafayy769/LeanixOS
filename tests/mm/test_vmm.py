import pytest

pytestmark = pytest.mark.vmm


# Test # 1 
def test_get_kerneldir(runner):
    assert "PASSED*" in runner.send_serial("vmm_get_kerneldir")


# Test # 2
def test_get_currentdir(runner):
    assert "PASSED*" in runner.send_serial("vmm_get_currentdir")


# Test # 3
def test_create_space(runner):
    assert "PASSED*" in runner.send_serial("vmm_create_space")


# Test # 4
def test_switch_dir(runner):
    assert "PASSED*" in runner.send_serial("vmm_switch_dir")


# Test # 5
def test_create_pt(runner):
    assert "PASSED*" in runner.send_serial("vmm_create_pt")


# Test # 6
def test_map_basic(runner):
    assert "PASSED*" in runner.send_serial("vmm_map_basic")


# Test # 7
def test_get_phys(runner):
    assert "PASSED*" in runner.send_serial("vmm_get_phys")


# Test # 8
def test_init(runner):
    assert "PASSED*" in runner.send_serial("vmm_init")


# Test # 9 
def test_page_alloc(runner):
    assert "PASSED*" in runner.send_serial("vmm_page_alloc")


# Test # 10
def test_page_free(runner):
    assert "PASSED*" in runner.send_serial("vmm_page_free")


# Test # 11
def test_alloc_region(runner):
    assert "PASSED*" in runner.send_serial("vmm_alloc_region")


# Test # 12
def test_free_region(runner):
    assert "PASSED*" in runner.send_serial("vmm_free_region")


# Test # 13
def test_double_map(runner):
    assert "PASSED*" in runner.send_serial("vmm_double_map")


# Test # 14
def test_clone_pagetable(runner):
    assert "PASSED*" in runner.send_serial("vmm_clone_pagetable")


# Test # 15
def test_clone_dir(runner):
    assert "PASSED*" in runner.send_serial("vmm_clone_dir")

#--------------HIDDEN TESTS------------------------------

#Test # 16 
def test_memory_reuse_cycle(runner):
    assert "PASSED*" in runner.send_serial("vmm_memory_reuse_cycle")


#Test # 17
def test_page_table_cleanup (runner):
    assert "PASSED*" in runner.send_serial("vmm_page_table_cleanup")

#Test # 18
def test_rapid_remapping (runner):
    assert "PASSED*" in runner.send_serial("vmm_rapid_remapping")


#Test # 19
def test_partial_region_operations (runner):
    assert "PASSED*" in runner.send_serial("vmm_partial_region_operations")


#Test # 20 
def test_multiple_address_spaces_stress (runner):
    assert "PASSED*" in runner.send_serial("vmm_multiple_address_spaces_stress")
