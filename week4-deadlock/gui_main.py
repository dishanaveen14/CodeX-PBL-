"""gui_main.py - main window, dialogs and System Status tab. Owner: Team Lead.

Teammates' tabs (BankerTab, DeadlockTab, RAGTab, UtilizationTab) are plugged
in here. Every tab has refresh(); this window calls it after EVERY change.
"""
import json
import tkinter as tk
from datetime import datetime
from tkinter import ttk, messagebox, filedialog, scrolledtext, simpledialog

import samples
from state import SystemState
from detection import classify_state
from utilization import compute_utilization
from gui_banker import BankerTab
from gui_deadlock import DeadlockTab
from gui_rag import RAGTab
from gui_util import UtilizationTab

FONT = ('Segoe UI', 10)
FONT_BOLD = ('Segoe UI', 10, 'bold')
STATUS_COLORS = {          # state -> (background, text colour)
    'SAFE': ('#2e9e4f', 'white'),
    'UNSAFE': ('#e8890c', 'white'),
    'DEADLOCKED': ('#d32f2f', 'white'),
    'NO DATA': ('#8a8f98', 'white'),
    'UNKNOWN': ('#8a8f98', 'white'),
}
MENU = [
    '1. Create Processes', '2. Create Resources', '3. Enter Allocation Matrix',
    '4. Enter Maximum Matrix', '5. Display Need Matrix',
    '6. Display Resource Allocation Graph', '7. Request Resources',
    "8. Run Banker's Algorithm", '9. Detect Deadlock',
    '10. Recover from Deadlock', '11. Display System Status', '12. Exit',
]


# =====================================================================
#  System Status tab (menu 5 and 11)
# =====================================================================
class StatusTab(ttk.Frame):
    """Shows Total, Available, Allocation, Maximum, Need and Request tables."""

    def __init__(self, parent, app):
        super().__init__(parent)
        self.app = app
        top = ttk.Frame(self)
        top.pack(fill='x', padx=10, pady=(10, 4))
        self.total_var = tk.StringVar()
        self.avail_var = tk.StringVar()
        ttk.Label(top, textvariable=self.total_var, font=FONT_BOLD).pack(anchor='w')
        ttk.Label(top, textvariable=self.avail_var, font=FONT_BOLD,
                  foreground='#1565c0').pack(anchor='w', pady=(2, 0))

        grid = ttk.Frame(self)
        grid.pack(fill='both', expand=True, padx=10, pady=4)
        grid.columnconfigure((0, 1), weight=1)
        grid.rowconfigure((0, 1), weight=1)
        self.frames = {}
        self.trees = {}
        for idx, key in enumerate(('Allocation', 'Maximum', 'Need', 'Request')):
            title = {'Request': 'Pending Requests'}.get(key, key + ' Matrix')
            frame = tk.LabelFrame(grid, text=' %s ' % title, font=FONT_BOLD, padx=4, pady=4)
            frame.grid(row=idx // 2, column=idx % 2, sticky='nsew', padx=4, pady=4)
            tree = ttk.Treeview(frame, show='headings', height=6)
            tree.pack(fill='both', expand=True)
            self.frames[key] = frame
            self.trees[key] = tree

    def _fill(self, key, matrix):
        st = self.app.state
        tree = self.trees[key]
        cols = ['Process'] + st.resource_names
        tree.configure(columns=cols)
        for c in cols:
            tree.heading(c, text=c)
            tree.column(c, width=70, anchor='center', stretch=True)
        tree.delete(*tree.get_children())
        for i in range(st.n):
            label = st.process_names[i] + ('' if st.active[i] else ' (terminated)')
            tree.insert('', 'end', values=[label] + list(matrix[i]))

    def refresh(self):
        st = self.app.state
        self.total_var.set('Total resources:      %s' % self._vec(st.total))
        self.avail_var.set('Available vector:    %s' % self._vec(st.available()))
        self._fill('Allocation', st.allocation)
        self._fill('Maximum', st.maximum)
        self._fill('Need', st.need())
        self._fill('Request', st.request)

    def _vec(self, vec):
        st = self.app.state
        if not vec:
            return '(none yet)'
        return '   '.join('%s = %d' % (st.resource_names[j], v) for j, v in enumerate(vec))

    def highlight_need(self):
        """Flash the Need table so the user sees it (menu button 5)."""
        frame = self.frames['Need']
        old_fg, old_bg = frame.cget('fg'), frame.cget('bg')
        frame.configure(fg='#d32f2f', bg='#fff3cd')
        self.after(1800, lambda: frame.configure(fg=old_fg, bg=old_bg))


# =====================================================================
#  Main application window
# =====================================================================
class MainApp:
    """The main window. Tabs use: app.state, app.refresh_all(), app.log(), app.history."""

    def __init__(self, root):
        self.root = root
        self.state = SystemState()
        self.history = []          # [{'event', 'status', 'utilization'}]
        self.last_event = 'Start'
        self.status = 'NO DATA'

        root.title('Deadlock Simulation System')
        root.geometry('1180x740')
        root.minsize(980, 620)
        style = ttk.Style()
        try:
            style.theme_use('clam')
        except tk.TclError:
            pass
        style.configure('.', font=FONT)
        style.configure('Menu.TButton', font=FONT, padding=(10, 7), anchor='w')
        style.configure('TNotebook.Tab', padding=(12, 6), font=FONT_BOLD)

        self._build_menubar()
        self._build_banner()
        body = ttk.Frame(root)
        body.pack(fill='both', expand=True)
        self._build_sidebar(body)
        self._build_main_area(body)
        root.protocol('WM_DELETE_WINDOW', self.exit_app)
        self.log('Welcome! Use the buttons on the left, or File > Load sample.')
        self.refresh_all('Start')

    # ----------------------------------------------------------- layout
    def _build_menubar(self):
        bar = tk.Menu(self.root)
        filem = tk.Menu(bar, tearoff=0)
        sample_menu = tk.Menu(filem, tearoff=0)
        for name in ('safe', 'unsafe', 'deadlock'):
            sample_menu.add_command(label=name.capitalize() + ' scenario',
                                    command=lambda n=name: self.load_sample(n))
        filem.add_cascade(label='Load sample', menu=sample_menu)
        filem.add_command(label='Save scenario...', command=self.save_scenario)
        filem.add_command(label='Load scenario...', command=self.load_scenario)
        filem.add_separator()
        filem.add_command(label='Reset', command=self.reset)
        filem.add_command(label='Exit', command=self.exit_app)
        bar.add_cascade(label='File', menu=filem)
        helpm = tk.Menu(bar, tearoff=0)
        helpm.add_command(label='User guide', command=self.show_guide)
        helpm.add_command(label='About', command=lambda: messagebox.showinfo(
            'About', 'Interactive Deadlock Simulation System\nOperating Systems PBL - Week 4'))
        bar.add_cascade(label='Help', menu=helpm)
        self.root.config(menu=bar)

    def _build_banner(self):
        self.banner = tk.Label(self.root, text='SYSTEM STATE: NO DATA',
                               font=('Segoe UI', 16, 'bold'), pady=10)
        self.banner.pack(fill='x')

    def _build_sidebar(self, body):
        side = ttk.Frame(body, padding=8)
        side.pack(side='left', fill='y')
        ttk.Label(side, text='MENU', font=('Segoe UI', 11, 'bold')).pack(anchor='w', pady=(0, 6))
        actions = [self.create_processes, self.create_resources, self.enter_allocation,
                   self.enter_maximum, self.show_need, self.show_rag, self.request_resources,
                   self.run_banker, self.detect_deadlock, self.recover, self.show_status,
                   self.exit_app]
        for text, cmd in zip(MENU, actions):
            ttk.Button(side, text=text, style='Menu.TButton', width=32,
                       command=cmd).pack(fill='x', pady=2)

    def _build_main_area(self, body):
        right = ttk.Frame(body)
        right.pack(side='left', fill='both', expand=True, padx=(0, 8), pady=8)
        self.notebook = ttk.Notebook(right)
        self.notebook.pack(fill='both', expand=True)
        self.tabs = {
            'status': StatusTab(self.notebook, self),
            'banker': BankerTab(self.notebook, self),
            'deadlock': DeadlockTab(self.notebook, self),
            'rag': RAGTab(self.notebook, self),
            'util': UtilizationTab(self.notebook, self),
        }
        titles = {'status': 'System Status', 'banker': "Banker's",
                  'deadlock': 'Deadlock & Recovery', 'rag': 'Resource Graph',
                  'util': 'Utilization'}
        for key, tab in self.tabs.items():
            self.notebook.add(tab, text=titles[key])
        ttk.Label(right, text='Activity log', font=FONT_BOLD).pack(anchor='w', pady=(8, 2))
        self.log_box = scrolledtext.ScrolledText(right, height=8, state='disabled',
                                                 font=('Consolas', 9), wrap='word')
        self.log_box.pack(fill='x')

    # -------------------------------------------------- shared services
    def log(self, text):
        """Append a timestamped line to the activity log."""
        self.log_box.configure(state='normal')
        self.log_box.insert('end', '[%s] %s\n' % (datetime.now().strftime('%H:%M:%S'), text))
        self.log_box.see('end')
        self.log_box.configure(state='disabled')

    def refresh_all(self, event=None):
        """Redraw every tab, update the banner, record history."""
        if event:
            self.last_event = event
        for key, tab in self.tabs.items():
            try:
                tab.refresh()
            except Exception as exc:                       # a teammate's bug must not crash us
                self.log('ERROR in %s tab refresh: %s' % (key, exc))
        self._update_status()

    def _update_status(self):
        st = self.state
        if st.n == 0 or st.m == 0:
            self.status = 'NO DATA'
        else:
            try:
                self.status = str(classify_state(st)).upper()
            except Exception as exc:
                self.status = 'UNKNOWN'
                self.log('ERROR in classify_state: %s' % exc)
        bg, fg = STATUS_COLORS.get(self.status, STATUS_COLORS['UNKNOWN'])
        self.banner.configure(text='SYSTEM STATE: %s' % self.status, bg=bg, fg=fg)
        try:
            util = float(compute_utilization(st).get('overall_percent', 0.0))
        except Exception:
            util = 0.0
        self.history.append({'event': self.last_event, 'status': self.status,
                             'utilization': util})
        del self.history[:-200]

    def _select(self, key):
        self.notebook.select(self.tabs[key])
        return self.tabs[key]

    def _call_tab(self, key, method):
        """Select a tab and call one of its methods if the teammate has written it."""
        tab = self._select(key)
        fn = getattr(tab, method, None)
        if fn is None:
            self.log('The %s tab has no %s() yet - the teammate is still working on it.' % (key, method))
        else:
            fn()

    def _need_data(self, need_matrix=False):
        st = self.state
        if st.n == 0 or st.m == 0:
            messagebox.showwarning('No data', 'Create processes (1) and resources (2) first,\n'
                                              'or use File > Load sample.')
            return False
        return True

    # ----------------------------------------------------- menu actions
    def create_processes(self):
        n = simpledialog.askinteger('Create Processes', 'Number of processes (1-10):',
                                    minvalue=1, maxvalue=10, parent=self.root)
        if n is None:
            return
        st = self.state
        if st.n and (any(any(r) for r in st.allocation) or any(any(r) for r in st.maximum)):
            if not messagebox.askyesno('Confirm', 'This clears the Allocation, Maximum and '
                                                  'Request matrices. Continue?'):
                return
        st.resize(n, st.m, keep_total=True)
        self.log('Created %d processes (%s..%s).' % (n, st.process_names[0], st.process_names[-1]))
        self.refresh_all('Created %d processes' % n)

    def create_resources(self):
        m = simpledialog.askinteger('Create Resources', 'Number of resource types (1-6):',
                                    minvalue=1, maxvalue=6, parent=self.root)
        if m is None:
            return
        st = self.state
        labels = [chr(ord('A') + j) for j in range(m)]
        old_total = (st.total + [0] * m)[:m]

        def on_ok(values):
            if st.m and (any(any(r) for r in st.allocation) or any(any(r) for r in st.maximum)):
                if not messagebox.askyesno('Confirm', 'This clears the Allocation, Maximum and '
                                                      'Request matrices. Continue?', parent=self.root):
                    return False
            st.resize(st.n, m)
            st.total = values[0]
            self.log('Created %d resource types (%s) with totals %s.' % (
                m, ', '.join(labels), st.total))
            self.refresh_all('Created %d resource types' % m)
            return True

        self._matrix_dialog('Create Resources - total instances (0-20)', [old_total],
                            ['Total'], labels, on_ok, max_value=20, validate=False)

    def enter_allocation(self):
        if not self._need_data():
            return
        st = self.state

        def on_ok(values):
            trial = st.copy()
            trial.allocation = values
            errors = trial.validate()
            if errors:
                messagebox.showerror('Invalid allocation', '\n'.join(errors[:8]), parent=self.root)
                return False
            st.allocation = values
            self.log('Allocation matrix updated.')
            self.refresh_all('Allocation matrix entered')
            return True

        self._matrix_dialog('Enter Allocation Matrix', st.allocation, st.process_names,
                            st.resource_names, on_ok)

    def enter_maximum(self):
        if not self._need_data():
            return
        st = self.state

        def on_ok(values):
            trial = st.copy()
            trial.maximum = values
            errors = trial.validate()
            if errors:
                messagebox.showerror('Invalid maximum', '\n'.join(errors[:8]), parent=self.root)
                return False
            st.maximum = values
            self.log('Maximum matrix updated.')
            self.refresh_all('Maximum matrix entered')
            return True

        self._matrix_dialog('Enter Maximum Matrix', st.maximum, st.process_names,
                            st.resource_names, on_ok)

    def show_need(self):
        tab = self._select('status')
        tab.highlight_need()
        self.log('Need = Maximum - Allocation (shown in the System Status tab).')

    def show_rag(self):
        self._select('rag')
        self.log('Showing the Resource Allocation Graph.')

    def request_resources(self):
        self._call_tab('banker', 'focus_request')

    def run_banker(self):
        self._call_tab('banker', 'run_banker')

    def detect_deadlock(self):
        self._call_tab('deadlock', 'detect')

    def recover(self):
        self._call_tab('deadlock', 'recover')

    def show_status(self):
        self._select('status')

    def exit_app(self):
        if messagebox.askyesno('Exit', 'Close the simulator?'):
            self.root.destroy()

    # ------------------------------------------------------- File menu
    def load_sample(self, name):
        self.state = samples.load_sample(name)
        self.log('Loaded the %s sample scenario.' % name)
        self.refresh_all('Loaded %s sample' % name)

    def reset(self):
        if messagebox.askyesno('Reset', 'Delete all data and start over?'):
            self.state = SystemState()
            self.history.clear()
            self.log('System reset.')
            self.refresh_all('Reset')

    def save_scenario(self):
        path = filedialog.asksaveasfilename(defaultextension='.json',
                                            filetypes=[('Scenario', '*.json')])
        if not path:
            return
        try:
            with open(path, 'w') as fh:
                json.dump(self.state.to_dict(), fh, indent=2)
            self.log('Saved scenario to %s' % path)
        except OSError as exc:
            messagebox.showerror('Save failed', str(exc))

    def load_scenario(self):
        path = filedialog.askopenfilename(filetypes=[('Scenario', '*.json')])
        if not path:
            return
        try:
            with open(path) as fh:
                st = SystemState.from_dict(json.load(fh))
        except (OSError, ValueError, KeyError, TypeError) as exc:
            messagebox.showerror('Load failed', 'Not a valid scenario file:\n%s' % exc)
            return
        self.state = st
        self.log('Loaded scenario from %s' % path)
        self.refresh_all('Loaded scenario file')

    def show_guide(self):
        messagebox.showinfo('User guide',
                            '1. Create Processes, 2. Create Resources.\n'
                            '3/4. Enter the Allocation and Maximum matrices.\n'
                            '5/11. See Need and the whole system status.\n'
                            '7/8. Request resources / run the Banker\'s Algorithm.\n'
                            '9/10. Detect and recover from deadlock.\n'
                            '6. See the Resource Allocation Graph.\n\n'
                            'Tip: File > Load sample gives ready-made examples.')

    # -------------------------------------------------- matrix dialog
    def _matrix_dialog(self, title, matrix, row_labels, col_labels, on_ok,
                       max_value=999, validate=True):
        """Modal grid of Entry boxes. on_ok(values) returns True to close the dialog."""
        win = tk.Toplevel(self.root)
        win.title(title)
        win.transient(self.root)
        win.resizable(False, False)
        frame = ttk.Frame(win, padding=14)
        frame.pack()
        ttk.Label(frame, text=title, font=('Segoe UI', 11, 'bold')).grid(
            row=0, column=0, columnspan=len(col_labels) + 1, pady=(0, 8))
        for j, c in enumerate(col_labels):
            ttk.Label(frame, text=c, font=FONT_BOLD).grid(row=1, column=j + 1)
        entries = []
        for i, r in enumerate(row_labels):
            ttk.Label(frame, text=r, font=FONT_BOLD).grid(row=i + 2, column=0, padx=6, pady=2)
            row_entries = []
            for j in range(len(col_labels)):
                e = ttk.Entry(frame, width=6, justify='center')
                e.insert(0, str(matrix[i][j]))
                e.grid(row=i + 2, column=j + 1, padx=3, pady=2)
                row_entries.append(e)
            entries.append(row_entries)

        def submit():
            values = []
            for row_entries in entries:
                row = []
                for e in row_entries:
                    text = e.get().strip()
                    if not text.isdigit() or int(text) > max_value:
                        messagebox.showerror('Invalid input',
                                             'Please enter whole numbers from 0 to %d.' % max_value,
                                             parent=win)
                        e.focus_set()
                        return
                    row.append(int(text))
                values.append(row)
            if on_ok(values):
                win.destroy()

        btns = ttk.Frame(frame)
        btns.grid(row=len(row_labels) + 2, column=0, columnspan=len(col_labels) + 1, pady=(10, 0))
        ttk.Button(btns, text='OK', command=submit).pack(side='left', padx=4)
        ttk.Button(btns, text='Cancel', command=win.destroy).pack(side='left', padx=4)
        win.bind('<Return>', lambda _e: submit())
        win.bind('<Escape>', lambda _e: win.destroy())
        if entries:
            entries[0][0].focus_set()
        win.grab_set()
