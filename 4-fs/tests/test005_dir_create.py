from fisop_test import FisopTest
from utils.constants import STATUS_OK
from utils.functions import mount_point_cwd, read_file, run

class TestDirCreate(FisopTest):

    def test_mkdir(self):
        result = run("mkdir mi_nueva_carpeta")

        self.assertEqual("", result.stdout)
        self.assertEqual("", result.stderr)
        self.assertEqual(STATUS_OK, result.returncode)

        cd = run("cd mi_nueva_carpeta")
        self.assertEqual("", cd.stdout)
        self.assertEqual("", cd.stderr)
        self.assertEqual(STATUS_OK, cd.returncode)

    def test_mkdir_1_nest_level(self):
        result = run("mkdir -p aaa/bbb")

        self.assertEqual("", result.stdout)
        self.assertEqual("", result.stderr)
        self.assertEqual(STATUS_OK, result.returncode)

        cd = run("cd aaa")
        self.assertEqual("", cd.stdout)
        self.assertEqual("", cd.stderr)
        self.assertEqual(STATUS_OK, cd.returncode)

        cd2 = run("cd aaa/bbb")
        self.assertEqual("", cd2.stdout)
        self.assertEqual("", cd2.stderr)
        self.assertEqual(STATUS_OK, cd2.returncode)
