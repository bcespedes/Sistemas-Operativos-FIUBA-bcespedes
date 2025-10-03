from fisop_test import FisopTest
from utils.constants import STATUS_OK
from utils.functions import mount_point_cwd, read_file, run

class TestDirDelete(FisopTest):

    def test_rmdir(self):
        run("mkdir juancito")

        result = run("rmdir juancito")

        self.assertEqual("", result.stdout)
        self.assertEqual("", result.stderr)
        self.assertEqual(STATUS_OK, result.returncode)

        cd = run("cd juancito")

        self.assertEqual("", cd.stdout)
        self.assertEqual("/bin/sh: 1: cd: can't cd to juancito\n", cd.stderr)
        self.assertNotEqual(STATUS_OK, cd.returncode)

    def test_rm_with_files_inside(self):
        run("mkdir juancito")
        run("touch juancito/123.txt")

        result = run("rm -rf juancito")

        self.assertEqual("", result.stdout)
        self.assertEqual("", result.stderr)
        self.assertEqual(STATUS_OK, result.returncode)
    
    def test_rmdir_1_level_nest(self):
        run("mkdir -p diego/maradona")

        result = run("rmdir diego/maradona")

        self.assertEqual("", result.stdout)
        self.assertEqual("", result.stderr)
        self.assertEqual(STATUS_OK, result.returncode)

        cd = run("cd diego")

        self.assertEqual("", cd.stdout)
        self.assertEqual("", cd.stderr)
        self.assertEqual(STATUS_OK, cd.returncode)

        cd = run("cd diego/maradona")

        self.assertEqual("", cd.stdout)
        self.assertEqual("/bin/sh: 1: cd: can't cd to diego/maradona\n", cd.stderr)
        self.assertNotEqual(STATUS_OK, cd.returncode)

    def test_rmdir_1_level_nest_and_files_inside(self):
        run("mkdir -p diego/maradona")
        run("echo 'la mano de Dios' > diego/do_not_delete_me.txt")
        run("touch diego/maradona/10.txt")

        result = run("rm -rf diego/maradona")

        self.assertEqual("", result.stdout)
        self.assertEqual("", result.stderr)
        self.assertEqual(STATUS_OK, result.returncode)

        cat = run("cat diego/maradona/10.txt")

        self.assertEqual("", cat.stdout)
        self.assertEqual('cat: diego/maradona/10.txt: No such file or directory\n', cat.stderr)
        self.assertNotEqual(STATUS_OK, cat.returncode)

        cat2 = run("cat diego/do_not_delete_me.txt")

        self.assertEqual("la mano de Dios\n", cat2.stdout)
        self.assertEqual("", cat2.stderr)
