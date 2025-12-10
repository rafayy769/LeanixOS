import pytest

pytestmark = pytest.mark.hfs


def assert_passed(result: str):
    """Helper: test passes if no 'FAILED:' and contains 'PASSED'."""
    assert "PASSED" in result
    assert "FAILED:" not in result


# ============================================================================
# LEVEL 1: Basic Filesystem Operations
# ============================================================================

def test_01_format_mount(runner):
    """Test filesystem format and mount"""
    result = runner.send_serial("test_01_format_mount")
    assert_passed(result)


# ============================================================================
# LEVEL 2: Single Directory Operations
# ============================================================================

def test_02_single_directory(runner):
    """Test creating a single directory"""
    result = runner.send_serial("test_02_single_directory")
    assert_passed(result)


def test_03_nested_directories(runner):
    """Test creating nested directories"""
    result = runner.send_serial("test_03_nested_directories")
    assert_passed(result)


# ============================================================================
# LEVEL 3: Single File Operations
# ============================================================================

def test_04_single_file_create(runner):
    """Test creating a single file"""
    result = runner.send_serial("test_04_single_file_create")
    assert_passed(result)


def test_05_small_file_write_read(runner):
    """Test writing and reading small data (< 1 block)"""
    result = runner.send_serial("test_05_small_file_write_read")
    assert "Hello HFS!" in result
    assert "FAILED:" not in result


# ============================================================================
# LEVEL 4: Multiple Files in Same Directory
# ============================================================================

def test_06_multiple_files_same_dir(runner):
    """Test creating 30 files in one directory"""
    result = runner.send_serial("test_06_multiple_files_same_dir")
    assert "created:30" in result
    assert "FAILED:" not in result


def test_07_write_multiple_files(runner):
    """Test writing data to multiple files in same directory"""
    result = runner.send_serial("test_07_write_multiple_files")
    assert "File number 5" in result
    assert "FAILED:" not in result


# ============================================================================
# LEVEL 5: Files and Directories in Different Locations
# ============================================================================

def test_08_files_in_multiple_dirs(runner):
    """Test creating multiple directories with files in each"""
    result = runner.send_serial("test_08_files_in_multiple_dirs")
    assert "PASSED:4dirs_5files_each" in result
    assert "FAILED:" not in result


def test_09_complex_tree_structure(runner):
    """Test complex directory tree with files at various levels"""
    result = runner.send_serial("test_09_complex_tree_structure")
    assert "tree_files:7" in result
    assert "FAILED:" not in result


# ============================================================================
# LEVEL 6: Medium-Sized Files (Multiple Blocks, Direct Pointers Only)
# ============================================================================

def test_10_medium_file_direct_pointers(runner):
    """Test writing and reading 3KB file using direct pointers only"""
    result = runner.send_serial("test_10_medium_file_direct_pointers")
    assert "PASSED:3KB" in result
    assert "FAILED:" not in result


# ============================================================================
# LEVEL 7: Large Files (Requiring Indirect Pointers)
# ============================================================================

def test_11_large_file_indirect_pointer(runner):
    """Test 8KB file requiring indirect pointer block (exceeds 13*512=6656 bytes)"""
    result = runner.send_serial("test_11_large_file_indirect_pointer")
    assert "PASSED:8KB_indirect" in result
    assert "FAILED:" not in result


def test_12_very_large_file(runner):
    """Test very large file (20KB) with extensive indirect pointer usage"""
    result = runner.send_serial("test_12_very_large_file")
    assert "PASSED:blocks_written:" in result
    # Should have written at least 30 blocks
    blocks_str = result.split("blocks_written:")[1].split('*')[0].strip()
    blocks = int(blocks_str)
    assert blocks >= 30, f"Expected at least 30 blocks, got {blocks}"
    assert "FAILED:" not in result


def test_13_multiple_large_files(runner):
    """Test creating multiple large files (each requiring indirect pointers)"""
    result = runner.send_serial("test_13_multiple_large_files")
    assert "large_files:" in result
    # Should have created at least 3 large files
    files_str = result.split("large_files:")[1].split('*')[0].strip()
    files = int(files_str)
    assert files >= 3, f"Expected at least 3 large files, got {files}"
    assert "FAILED:" not in result


# ============================================================================
# LEVEL 8: File Modification Operations
# ============================================================================

def test_14_file_overwrite(runner):
    """Test overwriting file data"""
    result = runner.send_serial("test_14_file_overwrite")
    assert "Second write is longer!" in result
    assert "FAILED:" not in result


def test_15_write_at_offset(runner):
    """Test writing at specific offset"""
    result = runner.send_serial("test_15_write_at_offset")
    assert "AAAAABBBBB" in result
    assert "FAILED:" not in result


def test_16_partial_operations(runner):
    """Test partial reads and writes"""
    result = runner.send_serial("test_16_partial_operations")
    assert "PASSED:partial_ops" in result
    assert "FAILED:" not in result


# ============================================================================
# LEVEL 9: Path Lookup and Deep Nesting
# ============================================================================

def test_17_deep_path_lookup(runner):
    """Test deep path lookup through multiple directory levels"""
    result = runner.send_serial("test_17_deep_path_lookup")
    assert_passed(result)


def test_18_very_deep_nesting(runner):
    """Test very deep directory nesting (10 levels)"""
    result = runner.send_serial("test_18_very_deep_nesting")
    assert "depth:" in result
    # Should achieve at least 7 levels of nesting
    depth_str = result.split("depth:")[1].split('*')[0].strip()
    depth = int(depth_str)
    assert depth >= 7, f"Expected at least 7 levels of nesting, got {depth}"
    assert "FAILED:" not in result


# ============================================================================
# LEVEL 10: Stress Tests and Resource Allocation
# ============================================================================

def test_19_inode_allocation_stress(runner):
    """Test inode allocation stress (50 directories)"""
    result = runner.send_serial("test_19_inode_allocation_stress")
    assert "inodes:" in result
    # Should have allocated at least 30 inodes
    inodes_str = result.split("inodes:")[1].split('*')[0].strip()
    inodes = int(inodes_str)
    assert inodes >= 30, f"Expected at least 30 inodes, got {inodes}"
    assert "FAILED:" not in result


def test_20_block_allocation_stress(runner):
    """Test block allocation stress (20 files with 1KB each)"""
    result = runner.send_serial("test_20_block_allocation_stress")
    assert "files_with_blocks:" in result
    # Should have created at least 15 files
    files_str = result.split("files_with_blocks:")[1].split('*')[0].strip()
    files = int(files_str)
    assert files >= 15, f"Expected at least 15 files, got {files}"
    assert "FAILED:" not in result


def test_21_mixed_operations_stress(runner):
    """Test mixed operations stress (directories, files, writes)"""
    result = runner.send_serial("test_21_mixed_operations_stress")
    assert "operations:" in result
    # Should have completed at least 10 operations
    ops_str = result.split("operations:")[1].split('*')[0].strip()
    ops = int(ops_str)
    assert ops >= 10, f"Expected at least 10 operations, got {ops}"
    assert "FAILED:" not in result


# ============================================================================
# HIDDEN TEST 01: Sparse File Write with Random Offsets
# ============================================================================

def test_h01_sparse_file_random_offsets(runner):
    """Test sparse file writes at non-contiguous offsets"""
    result = runner.send_serial("test_h01_sparse_file_random_offsets")
    assert "PASSED:sparse_offsets" in result
    assert "FAILED:" not in result


# ============================================================================
# HIDDEN TEST 02: Simultaneous Read/Write on Multiple Files
# ============================================================================

def test_h02_interleaved_file_operations(runner):
    """Test interleaved operations across multiple open files"""
    result = runner.send_serial("test_h02_interleaved_file_operations")
    assert "PASSED:interleaved" in result
    assert "FAILED:" not in result


# ============================================================================
# HIDDEN TEST 03: Maximum File Size Test
# ============================================================================

def test_h03_maximum_file_size(runner):
    """Test writing 35KB file (70 blocks) to stress indirect pointers"""
    result = runner.send_serial("test_h03_maximum_file_size")
    assert "max_blocks:" in result
    # Should have written at least 50 blocks
    blocks_str = result.split("max_blocks:")[1].split('*')[0].strip()
    blocks = int(blocks_str)
    assert blocks >= 50, f"Expected at least 50 blocks, got {blocks}"
    assert "FAILED:" not in result


# ============================================================================
# HIDDEN TEST 04: Directory Entry Stress Test
# ============================================================================

def test_h04_directory_entry_stress(runner):
    """Test creating 40 files in a single directory"""
    result = runner.send_serial("test_h04_directory_entry_stress")
    assert "dir_entries:" in result
    # Should have created at least 25 files
    entries_str = result.split("dir_entries:")[1].split('*')[0].strip()
    entries = int(entries_str)
    assert entries >= 25, f"Expected at least 25 entries, got {entries}"
    assert "FAILED:" not in result


# ============================================================================
# HIDDEN TEST 05: Complex Overwrite with Expansion
# ============================================================================

def test_h05_complex_overwrite_expansion(runner):
    """Test complex overwrite scenarios across block boundaries"""
    result = runner.send_serial("test_h05_complex_overwrite_expansion")
    assert "PASSED:complex_expand" in result
    assert "FAILED:" not in result


# ============================================================================
# HIDDEN TEST 06: Deep Tree with Files at Each Level
# ============================================================================

def test_h06_deep_tree_with_files(runner):
    """Test 5-level deep directory tree with files at each level"""
    result = runner.send_serial("test_h06_deep_tree_with_files")
    assert "tree:" in result
    assert "levels:" in result
    assert "files" in result
    # Should have created at least 4 levels
    levels_str = result.split("tree:")[1].split("levels:")[0].strip()
    levels = int(levels_str)
    assert levels >= 4, f"Expected at least 4 levels, got {levels}"
    # Should have at least 8 files
    files_str = result.split("levels:")[1].split("files")[0].strip()
    files = int(files_str)
    assert files >= 8, f"Expected at least 8 files, got {files}"
    assert "FAILED:" not in result


# ============================================================================
# HIDDEN TEST 07: Fragmented File Write Pattern
# ============================================================================

def test_h07_fragmented_writes(runner):
    """Test alternating small/large writes to create fragmentation"""
    result = runner.send_serial("test_h07_fragmented_writes")
    assert "fragmented:" in result
    assert "bytes" in result
    # Should have written at least 8000 bytes
    bytes_str = result.split("fragmented:")[1].split("bytes")[0].strip()
    bytes_written = int(bytes_str)
    assert bytes_written >= 8000, f"Expected at least 8000 bytes, got {bytes_written}"
    assert "FAILED:" not in result


# ============================================================================
# HIDDEN TEST 08: Concurrent Large File Growth
# ============================================================================

def test_h08_concurrent_large_file_growth(runner):
    """Test growing 5 large files simultaneously in interleaved manner"""
    result = runner.send_serial("test_h08_concurrent_large_file_growth")
    assert "concurrent:" in result
    assert "files" in result
    # Should have successfully grown at least 4 files
    files_str = result.split("concurrent:")[1].split("files")[0].strip()
    files = int(files_str)
    assert files >= 4, f"Expected at least 4 files, got {files}"
    assert "FAILED:" not in result


# ============================================================================
# HIDDEN TEST 09: Cross-Boundary Edge Cases
# ============================================================================

def test_h09_cross_boundary_edge_cases(runner):
    """Test various read/write operations crossing block boundaries"""
    result = runner.send_serial("test_h09_cross_boundary_edge_cases")
    assert "PASSED:boundary_edges" in result
    assert "FAILED:" not in result


# ============================================================================
# HIDDEN TEST 10: Comprehensive Stress Test
# ============================================================================

def test_h10_comprehensive_stress_test(runner):
    """Test mixed operations at scale: directories, files, writes, random access"""
    result = runner.send_serial("test_h10_comprehensive_stress_test")
    assert "stress:dirs:" in result
    assert "files:" in result
    assert "ops:" in result
    
    # Parse results
    parts = result.split("stress:dirs:")[1]
    dirs_str = parts.split(":files:")[0].strip()
    dirs = int(dirs_str)
    
    files_str = parts.split(":files:")[1].split(":ops:")[0].strip()
    files = int(files_str)
    
    ops_str = parts.split(":ops:")[1].split('*')[0].strip()
    ops = int(ops_str)
    
    # Verify thresholds
    assert dirs >= 6, f"Expected at least 6 dirs, got {dirs}"
    assert files >= 15, f"Expected at least 15 files, got {files}"
    assert ops >= 25, f"Expected at least 25 operations, got {ops}"
    assert "FAILED:" not in result
    
    # Assertions
    assert dirs >= 8, f"Expected at least 8 directories, got {dirs}"
    assert files >= 20, f"Expected at least 20 files, got {files}"
    assert ops >= 30, f"Expected at least 30 operations, got {ops}"
    assert "FAILED:" not in result