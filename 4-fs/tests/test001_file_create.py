from fisop_test import FisopTest
from utils.constants import STATUS_OK
from utils.functions import mount_point_cwd, read_file, run

class TestFileCreate(FisopTest):

    def test_touch(self):
        result = run("touch file1.txt")
        self.assertEqual("", result.stdout)
        self.assertEqual("", result.stderr)
        self.assertEqual(STATUS_OK, result.returncode)
        self.assertEqual("", read_file("/file1.txt"))

    def test_redirection_stdout(self):
        result = run("echo 'hola' > file2.txt")
        self.assertEqual("", result.stdout)
        self.assertEqual("", result.stderr)
        self.assertEqual(STATUS_OK, result.returncode)
        self.assertEqual("hola\n", read_file("/file2.txt"))
    
    def test_redirection_stderr(self):
        result = run("aaaa 2> file3.txt")
        self.assertEqual("", result.stdout)
        self.assertEqual("", result.stderr)
        self.assertNotEqual(STATUS_OK, result.returncode)
        self.assertEqual("/bin/sh: 1: aaaa: not found\n", read_file("/file3.txt"))
    
    # CHALLENGE
    def test_hard_link(self):
        run("echo 'AABBCCDD 11223344' > old-patente.txt")
        
        created = run("ln old-patente.txt new-patente.txt")

        self.assertEqual("", created.stdout)
        self.assertEqual("", created.stderr)
        self.assertEqual(STATUS_OK, created.returncode)

        cat = run("cat new-patente.txt")

        self.assertEqual("AABBCCDD 11223344\n", cat.stdout)
        self.assertEqual("", cat.stderr)
        self.assertEqual(STATUS_OK, cat.returncode)

    # CHALLENGE
    def test_soft_link(self):
        run("echo 'hola' > aaaa.txt")
        
        sym = run("ln -s aaaa.txt link.txt")

        self.assertEqual("", sym.stdout)
        self.assertEqual("", sym.stderr)
        self.assertEqual(STATUS_OK, sym.returncode)

        cat = run("cat link.txt")

        self.assertEqual("hola\n", cat.stdout)
        self.assertEqual("", cat.stderr)
    