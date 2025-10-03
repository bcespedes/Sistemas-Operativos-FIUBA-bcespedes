from fisop_test import FisopTest
from utils.constants import STATUS_OK
from utils.functions import mount_point_cwd, read_file, run

class TestFileWrite(FisopTest):

    def test_truncate_to_zero(self):
        with open(f"{mount_point_cwd()}/numeros3.txt", "w") as file:
            for i in range(1, 33):
                file.write(str(i) + "\n")

        result = run("truncate -s 0 numeros3.txt")

        self.assertEqual("", result.stdout)
        self.assertEqual("", result.stderr)
        self.assertEqual(STATUS_OK, result.returncode)

        new_size = run("cat numeros3.txt")

        self.assertEqual("", new_size.stdout)
        self.assertEqual("", new_size.stderr)

    def test_truncate(self):
        with open(f"{mount_point_cwd()}/numeros4.txt", "w") as file:
            for i in range(1, 33):
                file.write(str(i) + "\n")

        result = run("truncate -s 8 numeros4.txt")

        self.assertEqual("", result.stdout)
        self.assertEqual("", result.stderr)
        self.assertEqual(STATUS_OK, result.returncode)

        new_size = run("cat numeros4.txt")

        self.assertEqual("1\n2\n3\n4\n", new_size.stdout)
        self.assertEqual("", new_size.stderr)
    
    def test_append(self):
        with open(f"{mount_point_cwd()}/numeros51.txt", "w") as file:
            for i in range(0, 5):
                file.write(str(i) + "\n")
        
        with open(f"{mount_point_cwd()}/numeros52.txt", "w") as file:
            for i in range(5, 10):
                file.write(str(i) + "\n")

        result = run("cat numeros52.txt >> numeros51.txt")

        self.assertEqual("", result.stdout)
        self.assertEqual("", result.stderr)
        self.assertEqual(STATUS_OK, result.returncode)

        new_content = run("cat numeros51.txt")

        self.assertEqual('0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n', new_content.stdout)
        self.assertEqual("", new_content.stderr)
