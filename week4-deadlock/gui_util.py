"""gui_util.py - STUB. Owner: Student 3 (replace this whole file with the real code)."""
from tkinter import ttk


class UtilizationTab(ttk.Frame):
    def __init__(self, parent, app):
        super().__init__(parent)
        self.app = app
        ttk.Label(self, text='Student 3 is working on this tab (Utilization)',
                  font=('Segoe UI', 14)).pack(expand=True)

    def refresh(self):
        """Redraw everything from self.app.state. Called after every change."""
        pass
