from fisop_test import FisopTest
from utils.constants import STATUS_OK
from utils.functions import mount_point_cwd, read_file, run

class TestFileDelete(FisopTest):

    def test_rm_file(self):
        run("touch pepe.txt")

        result = run("rm pepe.txt")

        self.assertEqual("", result.stdout)
        self.assertEqual("", result.stderr)
        self.assertEqual(STATUS_OK, result.returncode)

        exists = run("rm pepe.txt")

        self.assertEqual("", exists.stdout)
        self.assertEqual("rm: cannot remove 'pepe.txt': No such file or directory\n", exists.stderr)
        self.assertNotEqual(STATUS_OK, exists.returncode)
    
    def test_unlink_hard(self):
        run("echo 'SIS OPS' > old-pepe.txt")
        run("ln old-pepe.txt new-pepe.txt")

        unlinked = run("unlink old-pepe.txt")

        self.assertEqual("", unlinked.stdout)
        self.assertEqual("", unlinked.stderr)
        self.assertEqual(STATUS_OK, unlinked.returncode)

        # check file content still exists
        self.assertEqual("SIS OPS\n", read_file("/new-pepe.txt"))

        unlinked2 = run("unlink new-pepe.txt")

        self.assertEqual("", unlinked2.stdout)
        self.assertEqual("", unlinked2.stderr)
        self.assertEqual(STATUS_OK, unlinked2.returncode)

        cat_result = run("cat new-pepe.txt")

        self.assertEqual("", cat_result.stdout)
        self.assertEqual('cat: new-pepe.txt: No such file or directory\n', cat_result.stderr)
        self.assertNotEqual(STATUS_OK, cat_result.returncode)

    # CHALLENGE
    def test_unlink_soft(self):
        run("echo 'eaeaeaea' > jijo.txt")

        run("ln -s jijo.txt soft.txt")
        unlinked1 = run("unlink soft.txt")

        self.assertEqual("", unlinked1.stdout)
        self.assertEqual("", unlinked1.stderr)
        self.assertEqual(STATUS_OK, unlinked1.returncode)

        cat1 = run("cat jijo.txt")

        self.assertEqual("eaeaeaea\n", cat1.stdout)
        self.assertEqual("", cat1.stderr)
        self.assertEqual(STATUS_OK, cat1.returncode)

        run("ln -s jijo.txt soft.txt")
        unlinked2 = run("unlink jijo.txt")

        self.assertEqual("", unlinked2.stdout)
        self.assertEqual("", unlinked2.stderr)
        self.assertEqual(STATUS_OK, unlinked2.returncode)

        cat2 = run("cat soft.txt")

        self.assertEqual("", cat2.stdout)
        self.assertEqual('cat: soft.txt: No such file or directory\n', cat2.stderr)
        self.assertNotEqual(STATUS_OK, cat2.returncode)
