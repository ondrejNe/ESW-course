/*
 * Copyright (c) 2017, Red Hat Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 *  * Neither the name of Oracle nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */
package cz.cvut.fel.esw.nonblock.map;

import org.openjdk.jcstress.JCStress;
import org.openjdk.jcstress.Options;
import org.openjdk.jcstress.annotations.*;
import org.openjdk.jcstress.infra.results.I_Result;

/**
 * Concurrent behaviour testing with jcstress. It is not in the test folder due to complications with running it from
 * there.
 */
public class JcstressStringSetTest {

    static StringSet createSet() {
//        return new SynchronizedStringSet(minSize);
        return new NonblockStringSet(1);
    }

    @JCStressTest
    @Outcome(id = "0", expect = Expect.FORBIDDEN, desc = "Missed both writes.")
    @Outcome(id = "1", expect = Expect.FORBIDDEN, desc = "Missed write.")
    @Outcome(id = "2", expect = Expect.ACCEPTABLE, desc = "Default outcome.")
    @State
    public static class TwoDistinctTest {

        private final StringSet set = createSet(); //size one to enforce concurrent stress on a single bin

        @Actor
        public void actor1() {
            set.add("a");
        }

        @Actor
        public void actor2() {
            set.add("b");
        }

        @Arbiter
        public void size(I_Result r) {
            r.r1 = set.size();
        }
    }


    @JCStressTest
    @Outcome(id = "1", expect = Expect.ACCEPTABLE, desc = "Default outcome.")
    @Outcome(id = "2", expect = Expect.FORBIDDEN, desc = "Element duplicated.")
    @State
    public static class TwoEqualTest {

        private final StringSet set = createSet();

        @Actor
        public void actor1() {
            set.add("a");
        }

        @Actor
        public void actor2() {
            set.add("a");
        }

        @Arbiter
        public void size(I_Result r) {
            r.r1 = set.size();
        }
    }

    @JCStressTest
    @Outcome(id = "2", expect = Expect.FORBIDDEN, desc = "Missed write")
    @Outcome(id = "3", expect = Expect.ACCEPTABLE, desc = "Default outcome.")
    @State
    public static class TwoDistinctNonEmptyTest {

        private final StringSet set = createSet();

        public TwoDistinctNonEmptyTest() {
            set.add("prefill");
        }


        @Actor
        public void actor1() {
            set.add("a");
        }

        @Actor
        public void actor2() {
            set.add("b");
        }

        @Arbiter
        public void size(I_Result r) {
            r.r1 = set.size();
        }
    }

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

    public static void main(String[] args) throws Exception {
        //This is NOT the entirely correct/usual way to run the tests. The tool is made to be compiled by maven and run from command line (see jcstress website)

        //You must run 'mvn compile' before you run this. Jcstress has to generate some code. If you did not make any changes to this class, it does not have to regenerate anything therefore the previous mvn compilation is sufficient.
        Options opts = new Options(args);
        opts.parse();
        JCStress jcs = new JCStress(opts);
        jcs.run();

    }
}
