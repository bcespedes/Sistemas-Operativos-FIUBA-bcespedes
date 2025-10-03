from fisop_test import FisopTest
from utils.constants import STATUS_OK
from utils.functions import get_today, run

class TestDirRead(FisopTest):

    def test_ls_empty(self):
        ls = run("ls")
        self.assertEqual("", ls.stdout)
        self.assertEqual("", ls.stderr)
        self.assertEqual(STATUS_OK, ls.returncode)

    def test_ls_with_mkdir(self):
        run("mkdir mis_archivos")

        ls = run("ls")
        self.assertTrue("mis_archivos" in ls.stdout)
    
    def test_ls_dash_al(self):
        run("touch pipo.txt")

        # -rw-rw-r-- 1 nestor nestor 7951 2025-06-11 13:40 pipo.txt
        ls = run("ls -al --time-style=long-iso pipo.txt")
        today = get_today()

        self.assertTrue("-rw-r--r--" in ls.stdout)
        self.assertTrue(today in ls.stdout)
        self.assertTrue("pipo.txt" in ls.stdout)

        #cleanup
        run("rm pipo.txt")

    def test_ls_with_mkdir_1_level_nest(self):
        run("mkdir -p leonel/messi")

        ls = run("ls")
        self.assertTrue("leonel" in ls.stdout)

        ls2 = run("ls leonel")
        self.assertTrue("messi" in ls2.stdout)

        ls3 = run("ls leonel/messi")
        self.assertEqual("", ls3.stdout)

        cd_ls = run("cd leonel/messi && cd ../.. && ls")
        self.assertTrue("leonel" in cd_ls.stdout)

    def test_ls_pseudo_directories(self):
        run("mkdir -p leonel/messi")

        cd_ls = run("cd leonel/messi && ls .")
        self.assertTrue("" in cd_ls.stdout)

        cd_ls_2 = run("cd leonel/messi && ls ..")
        self.assertTrue("messi" in cd_ls_2.stdout)

        cd_ls_3 = run("cd leonel/messi && ls ../..")
        self.assertTrue("leonel" in cd_ls_3.stdout)
