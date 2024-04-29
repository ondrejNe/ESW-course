package cz.cvut.fel.esw.nonblock.map;

import java.util.concurrent.atomic.AtomicReferenceArray;
import java.util.concurrent.atomic.AtomicReferenceFieldUpdater;

/**
 * TODO by me
 */
public class NonblockStringSet implements StringSet {

    private final int mask;

    private final AtomicReferenceArray<Node> bins;

    public NonblockStringSet(int minSize) {
        if (minSize <= 0) {
            throw new IllegalArgumentException("Size must be greater than 0");
        }
        int binsLength = Utils.smallestGreaterPowerOfTwo(minSize);
        this.mask = binsLength - 1;
        this.bins = new AtomicReferenceArray<>(binsLength);
    }

    @Override
    public void add(String word) {
        int binIndex = getBinIndex(word);
        if (bins.compareAndSet(binIndex, null, new Node(word))){
            return;
        }
        Node bin = bins.get(binIndex);
        while (bin != null) {
            if (bin.word.equals(word)) {
                return;
            }
            if (bin.compareAndSetNext(null, new Node(word))){
                return;
            }
            bin = bin.next;
        }
    }

    @Override
    public boolean contains(String word) {
        int binIndex = getBinIndex(word);
        for (Node n = bins.get(binIndex); n != null; n = n.next) {
            if (n.word.equals(word)) {
                return true;
            }
        }
        return false;
    }

    @Override
    public int size() {
        return calculateSize();
    }

    private int calculateSize() {
        int size = 0;
        for (int i = 0; i < bins.length(); i++) {
            Node bin = bins.get(i);
            while (bin != null) {
                size++;
                bin = bin.next;
            }
        }
        return size;
    }

    private int getBinIndex(String word) {
        return word.hashCode() & mask;
    }

    private static class Node {

        private final String word;
        private volatile Node next;

        static final AtomicReferenceFieldUpdater<Node, Node> nextUpdater =
                AtomicReferenceFieldUpdater.newUpdater(Node.class, Node.class, "next");

        public Node(String word) {
            this.word = word;
            this.next = null;
        }

        public boolean compareAndSetNext(Node expect, Node update) {
            return nextUpdater.compareAndSet(this, expect, update);
        }
    }
}
