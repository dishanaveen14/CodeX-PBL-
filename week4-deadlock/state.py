"""state.py - the shared system state. Owner: Team Lead.

Everybody's code reads/changes the system through this class.
Do NOT change names or behaviour without telling the Team Lead.
"""
import copy


class SystemState:
    """Holds processes, resources and the matrices of the simulation."""

    def __init__(self, n=0, m=0):
        self.total = []
        self.resize(n, m)

    # ------------------------------------------------------------------ setup
    def resize(self, n, m, keep_total=False):
        """Reset to n processes and m resource types (all matrices become zero)."""
        old_total = list(self.total)
        self.n = n
        self.m = m
        self.process_names = ['P%d' % i for i in range(n)]
        self.resource_names = [chr(ord('A') + j) for j in range(m)]
        if keep_total:
            self.total = (old_total + [0] * m)[:m]
        else:
            self.total = [0] * m
        self.allocation = [[0] * m for _ in range(n)]
        self.maximum = [[0] * m for _ in range(n)]
        self.request = [[0] * m for _ in range(n)]
        self.active = [True] * n

    # --------------------------------------------------------------- computed
    def need(self):
        """Need = Maximum - Allocation (never below 0; all zeros if inactive)."""
        result = []
        for i in range(self.n):
            if not self.active[i]:
                result.append([0] * self.m)
            else:
                result.append([max(0, self.maximum[i][j] - self.allocation[i][j])
                               for j in range(self.m)])
        return result

    def available(self):
        """Available = Total - sum of allocations of active processes."""
        avail = list(self.total)
        for i in range(self.n):
            if self.active[i]:
                for j in range(self.m):
                    avail[j] -= self.allocation[i][j]
        return avail

    def copy(self):
        """Deep copy - use it for what-if checks."""
        return copy.deepcopy(self)

    # -------------------------------------------------------------- changes
    def grant(self, pid, vec):
        """Give vec to process pid and clear its pending request."""
        for j in range(self.m):
            self.allocation[pid][j] += vec[j]
        self.request[pid] = [0] * self.m

    def set_pending(self, pid, vec):
        """Process pid is now blocked, waiting for vec."""
        self.request[pid] = list(vec)

    def clear_pending(self, pid):
        self.request[pid] = [0] * self.m

    def terminate(self, pid):
        """Kill process pid: free everything it holds."""
        self.allocation[pid] = [0] * self.m
        self.request[pid] = [0] * self.m
        self.active[pid] = False

    def preempt(self, pid, vec):
        """Take vec back from process pid (never below zero)."""
        for j in range(self.m):
            self.allocation[pid][j] = max(0, self.allocation[pid][j] - vec[j])

    # ------------------------------------------------------------ validation
    def validate(self):
        """Return a list of error messages ([] means everything is OK).

        Checks that involve Maximum are skipped for a process whose Maximum
        row is still all zeros (the user has not entered it yet).
        """
        errors = []
        for i in range(self.n):
            for j in range(self.m):
                for label, matrix in (('Allocation', self.allocation),
                                      ('Maximum', self.maximum),
                                      ('Request', self.request)):
                    if matrix[i][j] < 0:
                        errors.append('%s[%s][%s] is negative.' % (
                            label, self.process_names[i], self.resource_names[j]))
        for j in range(self.m):
            held = sum(self.allocation[i][j] for i in range(self.n) if self.active[i])
            if held > self.total[j]:
                errors.append('Resource %s: allocated %d but total is only %d.' % (
                    self.resource_names[j], held, self.total[j]))
        for i in range(self.n):
            if not any(self.maximum[i]):
                continue
            for j in range(self.m):
                if self.maximum[i][j] > self.total[j]:
                    errors.append('%s needs max %d of %s but total is only %d.' % (
                        self.process_names[i], self.maximum[i][j],
                        self.resource_names[j], self.total[j]))
                if self.maximum[i][j] < self.allocation[i][j]:
                    errors.append('%s: Maximum of %s (%d) is below Allocation (%d).' % (
                        self.process_names[i], self.resource_names[j],
                        self.maximum[i][j], self.allocation[i][j]))
                if self.request[i][j] > self.maximum[i][j] - self.allocation[i][j]:
                    errors.append('%s: pending request for %s exceeds its Need.' % (
                        self.process_names[i], self.resource_names[j]))
        return errors

    # ------------------------------------------------------------------ JSON
    def to_dict(self):
        return {'n': self.n, 'm': self.m,
                'process_names': self.process_names,
                'resource_names': self.resource_names,
                'total': self.total, 'allocation': self.allocation,
                'maximum': self.maximum, 'request': self.request,
                'active': self.active}

    @classmethod
    def from_dict(cls, data):
        st = cls(int(data['n']), int(data['m']))
        st.process_names = list(data['process_names'])
        st.resource_names = list(data['resource_names'])
        st.total = [int(x) for x in data['total']]
        st.allocation = [[int(x) for x in row] for row in data['allocation']]
        st.maximum = [[int(x) for x in row] for row in data['maximum']]
        st.request = [[int(x) for x in row] for row in data['request']]
        st.active = [bool(x) for x in data['active']]
        return st
