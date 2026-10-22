"""gui_deadlock.py - STUB. Owner: Student 2 (replace this whole file with the real code)."""
from tkinter import ttk


class DeadlockTab(ttk.Frame):
    def __init__(self, parent, app):
        super().__init__(parent)
        self.app = app
        ttk.Label(self, text='Student 2 is working on this tab (Deadlock Detection and Recovery)',
                  font=('Segoe UI', 14)).pack(expand=True)

    def refresh(self):
        """Redraw everything from self.app.state. Called after every change."""
        pass
