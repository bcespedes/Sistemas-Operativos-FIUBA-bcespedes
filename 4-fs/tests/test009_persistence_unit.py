from fisop_test import FisopTest
from utils.constants import STATUS_OK
from utils.functions import mount_point_cwd, read_file, run


FISOPFS_CWD = f"{mount_point_cwd()}/../.."
FISOPFS_C_FILES = f"{FISOPFS_CWD}/fs_utils.c"

TEST_CWD = f"{mount_point_cwd()}/../utils"
TEST_NAME = "persistence_test"

FLAGS = "-Wall -Wconversion -Wtype-limits -Werror"
OUTPUT = f"{TEST_CWD}/{TEST_NAME}"
C_FILE = f"{TEST_CWD}/{TEST_NAME}.c"

GCC_COMPILE_TEST = f"gcc -std=c99 {FLAGS} -o {OUTPUT} {FISOPFS_C_FILES} {C_FILE}"


def gcc_compile():
    compiled = run(GCC_COMPILE_TEST)

    if (compiled.returncode != STATUS_OK):
        run(f"rm {OUTPUT}") #cleanup
        raise Exception(compiled.stderr)


class TestPersistenceUnit(FisopTest):

    def test_persistence_functions(self):
        gcc_compile()

        result = run(f"./{TEST_NAME}", cwd=TEST_CWD)

        try:
            self.assertEqual("", result.stderr)
            self.assertEqual(STATUS_OK, result.returncode)
            self.assertEqual("PERSISTENCE WRITE PASS\nPERSISTENCE READ PASS\n", result.stdout)
        except AssertionError as e:
            #cleanup
            run(f"rm {OUTPUT}")
            raise e

        #cleanup
        run(f"rm {OUTPUT}")
        run(f"rm {TEST_CWD}/test009.fisopfs") 
