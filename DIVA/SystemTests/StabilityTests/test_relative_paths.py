import os

this_dir = os.path.dirname(__file__)


def test(diva):
    deep_path = os.path.join(this_dir, '..', 'StabilityTests', 'HelloWorld.o')
    assert diva(deep_path, getelfs=False)
