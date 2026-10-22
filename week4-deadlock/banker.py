"""banker.py - STUB. Owner: Student 1 (replace this whole file with the real code)."""


def is_safe(state):
    """Return {'safe': bool, 'sequence': [names], 'steps': [text lines]}."""
    return {'safe': True, 'sequence': [], 'steps': ['TODO: Student 1 - safety algorithm']}


def check_request(state, pid, vec, use_banker=True):
    """Return {'status': 'GRANTED'|'EXCEEDS_NEED'|'MUST_WAIT'|'UNSAFE_DENIED',
    'message': str, 'safe_sequence': [...], 'steps': [...]}. Must NOT change state."""
    return {'status': 'MUST_WAIT', 'message': 'TODO: Student 1 - request algorithm',
            'safe_sequence': [], 'steps': []}
