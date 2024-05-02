package cz.cvut.fel.esw;

import java.util.Arrays;
import java.util.Random;
import java.util.random.RandomGenerator;

/**
 * @author Marek Cuchý (CVUT)
 */
public class Main {

    public static void main(String[] args) {
        RandomGenerator rnd = new Random(0);

        int[] arr1 = {1, 1, 1, 1, 1, 3};
        System.out.println("Array1 " + Arrays.toString(arr1));
        System.out.println("Sum " + Utils.sum(arr1));
        int[] arr2 = {2, 2, 2, 1, 1, 5};
        System.out.println("Array2 " + Arrays.toString(arr2));
        System.out.println("Dot " + Utils.dot(arr1, arr2));

        int n = 5;
        int m = 10;
        int p = 7;
        int ub = 10;

        Matrix left = Matrix.generateMatrix(rnd, n, m, ub);
        System.out.println(left);
        Matrix right = Matrix.generateMatrix(rnd, m, p, ub);
        System.out.println(right);
        System.out.println(Utils.multiply(left, right));
    }
}