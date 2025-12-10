import pytest
from runner import TestRunner

MONITOR_HOST      = "127.0.0.1"
SERIAL_HOST       = "127.0.0.1"
MONITOR_PORT      = 4444
SERIAL_PORT       = 5555
DEFAULT_TIMEOUT   = 10 # seconds
MAX_RETRIES       = 3 # times to retry connection

def pytest_configure(config):
    """Register custom markers"""
    config.addinivalue_line("markers", "vga: VGA display and cursor tests")
    config.addinivalue_line("markers", "interrupts: Interrupt handling tests")
    config.addinivalue_line("markers", "kbd: Keyboard input tests")
    config.addinivalue_line("markers", "tty: TTY display and input tests")
    config.addinivalue_line("markers", "shell: Shell command tests")
    config.addinivalue_line("markers", "sys: System call tests")
    config.addinivalue_line("markers", "kmm: kernel physical memory manager tests")
    config.addinivalue_line("markers", "kheap: kernel heap allocator tests")
    config.addinivalue_line("markers", "vmm: virtual memory manager tests")
    config.addinivalue_line("markers", "timer: PIT timer tests")
    config.addinivalue_line("markers", "tss: Task State Segment tests")
    config.addinivalue_line("markers", "elf: ELF loading tests")
    config.addinivalue_line("markers", "proc: process management tests")
    config.addinivalue_line("markers", "hfs: filesystem tests")

# CONFIGURE YOUR TEST SUITES HERE

ENABLED_SUITES = [
    "vga",
    "interrupts",
    "kbd",
    "tty",
    "shell",
    "sys",
    "kmm",
    "kheap",
    "vmm",
    "timer",
    "tss",
    "elf",
    "proc",
    "hfs"
]

def pytest_collection_modifyitems(config, items):
    """Filter and order tests based on ENABLED_SUITES"""
    
    # Filter: only keep tests that belong to enabled suites
    filtered_items = []
    for item in items:
        marks = [mark.name for mark in item.iter_markers()]
        # Check if test belongs to any enabled suite
        for suite in ENABLED_SUITES:
            if suite in marks:
                filtered_items.append(item)
                break
    
    # Sort filtered tests by suite order
    def get_priority(item):
        marks = [mark.name for mark in item.iter_markers()]
        for mark_name in marks:
            if mark_name in ENABLED_SUITES:
                return (ENABLED_SUITES.index(mark_name), item.nodeid)
        return (999, item.nodeid)
    
    filtered_items.sort(key=get_priority)
    
    # Replace the original items list
    items[:] = filtered_items

@pytest.fixture(scope="session")
def runner():
    
    r = TestRunner( serial_host=SERIAL_HOST,
                    serial_port=SERIAL_PORT,
                    monitor_host=MONITOR_HOST,
                    monitor_port=MONITOR_PORT,
                    default_timeout=DEFAULT_TIMEOUT,
                    retry_attempts=MAX_RETRIES )
    
    r.setup()
    yield r
    r.cleanup()
