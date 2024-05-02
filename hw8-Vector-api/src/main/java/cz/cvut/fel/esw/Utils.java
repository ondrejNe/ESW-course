package cz.cvut.fel.esw;

import jdk.incubator.vector.*;

/**
 * @author Marek Cuchý (CVUT)
 */
public class Utils {
    private static final VectorSpecies<Integer> SPECIES = IntVector.SPECIES_PREFERRED;
    public static int sum(int[] array) {
        int vectorSize = SPECIES.length();
        int arrayLength = array.length;
        int sum = 0;

        for (int i = 0; i < arrayLength; i += vectorSize) {
            VectorMask<Integer> mask = SPECIES.indexInRange(i, arrayLength);
            IntVector vector = IntVector.fromArray(SPECIES, array, i, mask);
            sum += vector.reduceLanes(VectorOperators.ADD, mask);
        }

        return sum;
    }

    public static int dot(int[] v1, int[] v2) {
        if (v1.length != v2.length) {
            throw new IllegalArgumentException("Vectors must be of the same length.");
        }

        int vectorSize = SPECIES.length();
        int arrayLength = v1.length;
        int sum = 0;

        for (int i = 0; i < arrayLength; i += vectorSize) {
            VectorMask<Integer> mask = SPECIES.indexInRange(i, arrayLength);
            IntVector vec1 = IntVector.fromArray(SPECIES, v1, i, mask);
            IntVector vec2 = IntVector.fromArray(SPECIES, v2, i, mask);
            IntVector result = vec1.mul(vec2, mask);
            sum += result.reduceLanes(VectorOperators.ADD, mask);
        }

        return sum;
    }

    public static Matrix multiply(Matrix left, Matrix right) {
        int[][] a = left.rowBased();
        int[][] bt = right.columnBased();
        int n = left.rows();
        int p = right.columns();
        int leftArrayLength = left.columns();

        int vectorSize = SPECIES.length();
        int[][] res = new int[n][p];

        for (int i = 0; i < n; i++) { // for each left row
            for (int j = 0; j < p; j++) { // for each right column
                var sum = 0;
                for (int k = 0; k < leftArrayLength; k += vectorSize) {
                    VectorMask<Integer> mask = SPECIES.indexInRange(k, leftArrayLength);
                    IntVector v1 = IntVector.fromArray(SPECIES, a[i], k, mask);
                    IntVector v2 = IntVector.fromArray(SPECIES, bt[j], k, mask);
                    IntVector product = v1.mul(v2, mask);
                    sum += product.reduceLanes(VectorOperators.ADD, mask);
                }
                res[i][j] = sum;
            }
        }
        return new Matrix(res);
    }
}
