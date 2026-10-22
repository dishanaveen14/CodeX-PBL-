"""samples.py - ready-made scenarios for testing. Owner: Team Lead."""
from state import SystemState


def load_sample(name):
    """Return a SystemState for 'safe', 'unsafe' or 'deadlock'."""
    if name in ('safe', 'unsafe'):
        st = SystemState(5, 3)
        st.total = [10, 5, 7]
        st.allocation = [[0, 1, 0], [2, 0, 0], [3, 0, 2], [2, 1, 1], [0, 0, 2]]
        st.maximum = [[7, 5, 3], [3, 2, 2], [9, 0, 2], [2, 2, 2], [4, 3, 3]]
        if name == 'unsafe':
            st.allocation[4] = [3, 3, 2]       # P4 additionally holds [3, 3, 0]
        return st
    if name == 'deadlock':
        st = SystemState(3, 3)
        st.total = [1, 1, 1]
        st.allocation = [[1, 0, 0], [0, 1, 0], [0, 0, 1]]
        st.maximum = [[1, 1, 0], [0, 1, 1], [1, 0, 1]]
        st.request = [[0, 1, 0], [0, 0, 1], [1, 0, 0]]
        return st
    raise ValueError('Unknown sample: %s' % name)
