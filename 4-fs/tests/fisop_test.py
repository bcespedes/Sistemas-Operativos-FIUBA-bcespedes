import os
import unittest
from utils.constants import MOUNT_POINT
from utils.functions import mount_point_cwd, run

# abstract class
class FisopTest(unittest.TestCase):

    # @BeforeAll
    # Configuration before running all tests from this class
    def setUpClass():
        os.chdir(mount_point_cwd())

    # @BeforeEach
    # Configuration before running each test method
    def setUp(self):
        print("\nTEST:", self._testMethodName + "()")
    
    # @AfterEach
    # Cleanup code after running each test method
    def tearDown(self):
        pass
        
    # @AfterAll
    # Cleanup code after running all tests from this class
    def tearDownClass():
        # delete all files and folders from tests/mount_point
        # (this command does not delete dot-files, such as .gitkeep)
        run("rm -R ./*")
