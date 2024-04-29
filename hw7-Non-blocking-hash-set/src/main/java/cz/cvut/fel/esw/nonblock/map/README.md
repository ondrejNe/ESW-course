# Non-blocking 

## What's added

- NonblockStringSet (whole class)
```java
// Atomic updates of an array, visible to multiple threads
private final AtomicReferenceArray<Node> bins;
// Compare and set for hash set updates
bins.compareAndSet(binIndex, null, new Node(word))
// Atomic reference field updater without using explicit locks or synchronization
// Volatile keyword so that the variable is visible to multiple threads
// and operations are atomic
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
```
- JcstressStringSetTest
```java
@JCStressTest
@Outcome(id = "6", expect = Expect.ACCEPTABLE, desc = "Default outcome.")
@State
public static class MultipleDistinctDuplicatesTest {

    private final StringSet set = createSet();

    public MultipleDistinctDuplicatesTest() {
        set.add("a");
        set.add("ab");
        set.add("c");
        set.add("aa");
    }

    @Actor
    public void actor1() {
        set.add("a");
    }

    @Actor
    public void actor2() {
        set.add("b");
    }

    @Actor
    public void actor3() {
        set.add("cc");
    }

    @Actor
    public void actor4() {
        set.add("aa");
    }

    @Arbiter
    public void size(I_Result r) {
        r.r1 = set.size();
    }
}
```
- VmlensStringSetTest
```java
@Test
public void testMultipleAddsRunner() throws InterruptedException {
    VmlensTestRunner runner = new VmlensTestRunner(testedSet);
    runner.initAdd("a");
    runner.initAdd("f");
    runner.taskAdd("b");
    runner.taskAdd("c");
    runner.initAdd("l");
    runner.taskAdd("c");
    runner.taskAdd("c");
    runner.taskAdd("c");
    runner.taskAdd("c");
    runner.taskAdd("c");
    runner.taskAdd("c");

    runner.assertContains("a");
    runner.assertContains("b");
    runner.assertContains("c");
    runner.assertSize(5);

    runner.executeVmlens();
}
```