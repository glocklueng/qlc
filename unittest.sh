#!/bin/bash

ARCH=`uname`

#############################################################################
# Engine tests
#############################################################################

TESTDIR=engine/test
TESTS=`find ${TESTDIR} -maxdepth 1 -mindepth 1 -type d`
for test in ${TESTS}
do
    # Ignore .svn
    if [ `echo ${test} | grep ".svn"` ]; then
        continue
    fi

    # Isolate just the test name
    test=`echo ${test} | sed 's/engine\/test\///'`

    # Execute the test
    pushd .
    cd ${TESTDIR}/${test}
    ./test.sh
    RESULT=${?}
    popd
    if [ ${RESULT} != 0 ]; then
        echo "${RESULT} Engine unit tests failed. Please fix before commit."
        exit ${RESULT}
    fi
done

#############################################################################
# UI tests
#############################################################################

TESTDIR=ui/test
TESTS=`find ${TESTDIR} -maxdepth 1 -mindepth 1 -type d`
for test in ${TESTS}
do
    # Ignore .svn
    if [ `echo ${test} | grep ".svn"` ]; then
        continue
    fi

    # Isolate just the test name
    test=`echo ${test} | sed 's/ui\/test\///'`

    # Execute the test
    pushd .
    cd ${TESTDIR}/${test}
    DYLD_FALLBACK_LIBRARY_PATH=$DYLD_FALLBACK_LIBRARY_PATH:../../../engine/src:../../src \
        LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../../engine/src:../../src ./${test}_test
    RESULT=${?}
    popd
    if [ ${RESULT} != 0 ]; then
        echo "${RESULT} UI unit tests failed. Please fix before commit."
        exit ${RESULT}
    fi
done

#############################################################################
# Enttec wing tests
#############################################################################

pushd .
cd plugins/ewinginput/test
sh test.sh
RESULT=$?
if [ $RESULT != 0 ]; then
	echo "${RESULT} Enttec wing unit tests failed. Please fix before commit."
	exit $RESULT
fi
popd

#############################################################################
# Velleman Output tests
#############################################################################

if [ ${ARCH} != "Darwin" ]; then
    pushd .
    cd plugins/vellemanout/test
    DYLD_FALLBACK_LIBRARY_PATH=$DYLD_FALLBACK_LIBRARY_PATH:../src \
	    LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../src ./vellemanout_test
    RESULT=$?
    if [ $RESULT != 0 ]; then
	    echo "Velleman Output unit test failed ($RESULT). Please fix before commit."
	    exit $RESULT
    fi
    popd
fi

#############################################################################
# MIDI Input tests
#############################################################################

pushd .
cd plugins/midiinput/common/test
DYLD_FALLBACK_LIBRARY_PATH=$DYLD_FALLBACK_LIBRARY_PATH:../src \
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../src ./common_test
RESULT=$?
if [ $RESULT != 0 ]; then
    echo "MIDI Input common unit test failed ($RESULT). Please fix before commit."
    exit $RESULT
fi
popd

#############################################################################
# Final judgment
#############################################################################

echo "Unit tests passed."
