import cz.cvut.fel.esw.Matrix;
import cz.cvut.fel.esw.Utils;
import org.junit.jupiter.api.Test;

import java.util.Random;
import java.util.random.RandomGenerator;

import static org.junit.jupiter.api.Assertions.assertEquals;

class UtilsTest {

    @Test
    void testSumFunctions() {
        int[] array = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        assertEquals(Utils.sumSequential(array), Utils.sumVector(array));
    }

    @Test
    void testDotFunctions() {
        int[] v1 = {1, 2, 3, 4, 5};
        int[] v2 = {1, 2, 3, 4, 5};
        assertEquals(Utils.dotSequential(v1, v2), Utils.dotVector(v1, v2));
    }

    @Test
    void testMatrixMultiplyFunctions() {
        RandomGenerator rnd = new Random(0);
        int n = 10;
        int m = 10;
        int p = 7;
        int ub = 100;
        Matrix left = Matrix.generateMatrix(rnd, n, m, ub);
        Matrix right = Matrix.generateMatrix(rnd, m, p, ub);

        Matrix resultOg = Utils.multiplySequential(left, right);
        Matrix resultVec = Utils.multiplyVector(left, right);

        for (int i = 0; i < resultOg.rows(); i++) {
            for (int j = 0; j < resultOg.columns(); j++) {
                assertEquals(resultOg.getValueAt(i, j), resultVec.getValueAt(i, j));
            }
        }
    }
}
