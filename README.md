# ScheduleManagement
VEDA Mini Project_1

## Event Function Naming Guide

### Recommended Convention

- Functions that open UI widgets: `show...Widget()`
- Functions that process data: `addEvent()`, `editEvent()`, `deleteEvent()`, `searchEvent()`
- Main window slots and handlers: `handle...()`

### Feature-Based Function Names

1. Add
- Open widget: `showAddEventWidget()`
- Process add: `addEvent()`
- Cancel: `cancelAddEvent()`

2. Edit
- Open widget: `showEditEventWidget(int eventId)`
- Process edit: `editEvent()`
- Cancel: `cancelEditEvent()`

3. Delete
- Open delete confirmation widget: `showDeleteEventWidget(int eventId)`
- Process delete: `deleteEvent(int eventId)`
- Cancel: `cancelDeleteEvent()`

4. Search
- Open search widget: `showSearchEventWidget()`
- Process search: `searchEvent(const QString &eventName)`
- Cancel: `cancelSearchEvent()`

### Mapping to Current Code

- `MainWindow::handleScheduleAdded(...)` -> `handleAddEvent(...)`
- `MainWindow::handleScheduleUpdated(...)` -> `handleEditEvent(...)`
- `MainWindow::handleScheduleDeleted(int id)` -> `handleDeleteEvent(int id)`
- `MainWindow::handleSearchRequested(...)` -> `handleSearchEvent(...)`

- `ScheduleListWidget::openCreateDialog()` -> `showAddEventWidget()`
- `ScheduleListWidget::openEditDialog(...)` -> `showEditEventWidget(...)`
- Delete confirmation `QMessageBox` section -> `showDeleteEventWidget(...)`
