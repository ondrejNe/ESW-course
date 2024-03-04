#!/bin/sh
set -eu
TT="timer: 4 timer: 5 timer: 4 timer: 4 timer: 5 timer: 4 timer: 5 timer: 4"
ST="7 4"
SST="7 0 0 3"
BIN=epoll_server

txt()
{
	echo $(cat $1)
}

res()
{
	if [ "$2" = "$(txt $3)" ]
	then
		echo "+ Good, $1 test passed"
	else
		echo "- Bad, $1 test failed."
		echo "    I do expect: '$2'"
		echo "    I found: '$(txt $3)'"
		ex=1
	fi
}

test -x $BIN && run=$BIN
test -x builddir/$BIN && run=builddir/$BIN
if [ -z "$run" ]
then
	echo "No $BIN binary found."
	exit 1
fi

tr=$(mktemp)
sr=$(mktemp)
ssr=$(mktemp)

(
	( sleep 2; echo -n "foo"; sleep 2; echo -n " bar\nbuzz\n"; ) \
	| ( sleep 1; nc localhost 12345 >$sr; )
)&
(
	(
		sleep 2; echo -n "this"; \
		sleep 1; echo -n " is\n"; \
		sleep 1; echo -n "\n\nesw\n"; \
		) \
	| ( sleep 1; nc localhost 12345 >$ssr; )
)&
timeout 6 ./$run | tee $tr

sed -i '/^[^(timer)].*/d' $tr

ex=0
res "timer" "$TT" "$tr"
res "1st socket" "$ST" "$sr"
res "2nd socket" "$SST" "$ssr"

rm $tr $sr $ssr
exit $ex
