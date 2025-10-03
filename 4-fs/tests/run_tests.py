import unittest
import warnings
from utils.functions import mount_point_cwd, run
from test001_file_create import TestFileCreate
from test002_file_read import TestFileRead
from test003_file_write import TestFileWrite
from test004_file_delete import TestFileDelete
from test005_dir_create import TestDirCreate
from test006_dir_read import TestDirRead
from test007_dir_delete import TestDirDelete
from test008_stat import TestStat
from test009_persistence_unit import TestPersistenceUnit

def suite():
    test_suite = unittest.TestSuite()
    for klass in [
        TestFileCreate,
        TestFileRead,
        TestFileWrite,
        TestFileDelete,
        TestDirCreate,
        TestDirRead,
        TestDirDelete,
        TestStat,
        TestPersistenceUnit
    ]:
        test_suite.addTest(unittest.makeSuite(klass))
    return test_suite

# DeprecationWarning: unittest.makeSuite() is deprecated and will be removed in Python 3.13. 
# Please use unittest.TestLoader.loadTestsFromTestCase() instead.
warnings.simplefilter('ignore', category=DeprecationWarning)

mySuit=suite()
runner=unittest.TextTestRunner()
runner.run(mySuit)

# un-mount the VFS
run(f"umount -l {mount_point_cwd()}")

# RUN TESTS
# STEP 1: 
#   umount -l tests/mount_point
# STEP 2: 
#   ./fisopfs -f -o nonempty tests/mount_point
# STEP 3 (IN ANOTHER CONSOLE): 
#   /bin/python3 tests/run_tests.py
