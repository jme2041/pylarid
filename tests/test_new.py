# pylarid: Load Arrays of Imaging Data
# Copyright 2023 Jeffrey Michael Engelmann

import pytest
import pylarid


@pytest.fixture
def blank_dset():
    return pylarid.Dset()


def test_good_new(blank_dset):
    obj2 = blank_dset.__new__(pylarid.Dset)
    assert obj2 is not None


def test_bad_new(blank_dset):
    with pytest.raises(TypeError):
        blank_dset.__new__(pylarid.Dset, 1)


def test_good_init(blank_dset):
    blank_dset.__init__()


def test_bad_init(blank_dset):
    with pytest.raises(TypeError):
        blank_dset.__init__(1)
