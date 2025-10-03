from fisop_test import FisopTest
from utils.constants import STATUS_OK
from utils.functions import mount_point_cwd, run

class TestFileRead(FisopTest):

    def test_cat(self):
        with open(f"{mount_point_cwd()}/mensaje.txt", "w") as file:
            file.write("hola mundo!")

        result = run("cat mensaje.txt")

        self.assertEqual("hola mundo!", result.stdout)
        self.assertEqual("", result.stderr)

    def test_more(self):
        with open(f"{mount_point_cwd()}/numeros1.txt", "w") as file:
            for i in range(1, 33):
                file.write(str(i) + "\n")

        result = run("more +27 numeros1.txt")

        self.assertEqual("27\n28\n29\n30\n31\n32\n", result.stdout)
        self.assertEqual("", result.stderr)

    def test_less(self):
        result = run("seq 5 | less")

        self.assertEqual("1\n2\n3\n4\n5\n", result.stdout)
        self.assertEqual("", result.stderr)
