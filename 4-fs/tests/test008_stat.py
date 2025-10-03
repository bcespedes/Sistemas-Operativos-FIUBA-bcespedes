from fisop_test import FisopTest
from utils.constants import STATUS_OK
from utils.functions import get_today, run

class TestStat(FisopTest):

    def test_stat(self):
        run("touch abcde.txt")

        stat = run("stat abcde.txt")
        today = get_today()

        self.assertTrue("File: abcde.txt" in stat.stdout)
        self.assertTrue("Size: 0" in stat.stdout)
        self.assertTrue("regular empty file" in stat.stdout)
        self.assertTrue("Links: 1" in stat.stdout)
        self.assertTrue("Access: (0644/-rw-r--r--)" in stat.stdout)
        self.assertTrue("Access: " + today in stat.stdout)
        self.assertTrue("Modify: " + today in stat.stdout)
        self.assertTrue("Change: " + today in stat.stdout)

        self.assertEqual("", stat.stderr)
        self.assertEqual(STATUS_OK, stat.returncode)
