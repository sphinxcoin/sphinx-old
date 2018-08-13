#!/bin/bash

TOPDIR=${TOPDIR:-$(git rev-parse --show-toplevel)}
SRCDIR=${SRCDIR:-$TOPDIR/src}
MANDIR=${MANDIR:-$TOPDIR/doc/man}

SPHINXCOIND=${SPHINXCOIND:-$SRCDIR/sphinxcoind}
SPHINXCOINCLI=${SPHINXCOINCLI:-$SRCDIR/sphinxcoin-cli}
SPHINXCOINTX=${SPHINXCOINTX:-$SRCDIR/sphinxcoin-tx}
SPHINXCOINQT=${SPHINXCOINQT:-$SRCDIR/qt/sphinxcoin-qt}

[ ! -x $SPHINXCOIND ] && echo "$SPHINXCOIND not found or not executable." && exit 1

# The autodetected version git tag can screw up manpage output a little bit
BTCVER=($($SPHINXCOINCLI --version | head -n1 | awk -F'[ -]' '{ print $6, $7 }'))

# Create a footer file with copyright content.
# This gets autodetected fine for sphinxcoind if --version-string is not set,
# but has different outcomes for sphinxcoin-qt and sphinxcoin-cli.
echo "[COPYRIGHT]" > footer.h2m
$SPHINXCOIND --version | sed -n '1!p' >> footer.h2m

for cmd in $SPHINXCOIND $SPHINXCOINCLI $SPHINXCOINTX $SPHINXCOINQT; do
  cmdname="${cmd##*/}"
  help2man -N --version-string=${BTCVER[0]} --include=footer.h2m -o ${MANDIR}/${cmdname}.1 ${cmd}
  sed -i "s/\\\-${BTCVER[1]}//g" ${MANDIR}/${cmdname}.1
done

rm -f footer.h2m