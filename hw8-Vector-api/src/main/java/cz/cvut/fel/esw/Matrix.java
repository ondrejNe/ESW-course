package cz.cvut.fel.esw;

import java.util.Arrays;
import java.util.random.RandomGenerator;

/**
 * @author Marek Cuchý (CVUT)
 */
public record Matrix(int[][] rowBased, int[][] columnBased) {

    public Matrix(int[][] rowBased) {
        this(rowBased, toColumnBased(rowBased));
    }

    public int rows() {
        return rowBased.length;
    }

    public int columns() {
        return rowBased[0].length;
    }

    public int getValueAt(int row, int column) {
        return rowBased[row][column];
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("Matrix{");

        sb.append("rowBased=");
        sb.append(System.lineSeparator());
        for (int[] row : rowBased) {
            sb.append(Arrays.toString(row));
            sb.append(System.lineSeparator());
        }

        sb.append(", columnBased=");
        sb.append(System.lineSeparator());
        for (int[] row : columnBased) {
            sb.append(Arrays.toString(row));
            sb.append(System.lineSeparator());
        }

        sb.append('}');
        return sb.toString();
    }

    private static int[][] toColumnBased(int[][] rowBased) {
        int[][] columnBased = new int[rowBased[0].length][rowBased.length];
        for (int i = 0; i < rowBased.length; i++) {
            for (int j = 0; j < rowBased[0].length; j++) {
                columnBased[j][i] = rowBased[i][j];
            }
        }
        return columnBased;
    }


    /**
     * Generate random {@code n*m} matrix with values within interval {@code [0,ub)}
     */
    public static Matrix generateMatrix(RandomGenerator rnd, int n, int m, int ub) {
        int[][] rowBased = new int[n][m];
        int[][] columnBased = new int[m][n];

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int val = rnd.nextInt(ub);
                rowBased[i][j] = val;
                columnBased[j][i] = val;
            }
        }
        return new Matrix(rowBased, columnBased);
    }
}
