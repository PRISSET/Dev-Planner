import os

application = defines.get('app', 'dist/Dev Planner.app')
appname = os.path.basename(application)

format = defines.get('format', 'UDBZ')
size = defines.get('size', None)
files = [application]
symlinks = {'Applications': '/Applications'}

icon = 'icon.icns'
badge_icon = icon

icon_locations = {
    appname: (140, 120),
    'Applications': (500, 120)
}

background = 'builtin-arrow'

show_status_bar = False
show_tab_view = False
show_toolbar = False
show_pathbar = False
show_sidebar = False
sidebar_width = 180

window_rect = ((200, 120), (660, 400))

default_view = 'icon-view'

show_icon_preview = False

arrange_by = None
grid_offset = (0, 0)
grid_spacing = 100
scroll_position = (0, 0)
label_pos = 'bottom'
text_size = 12
icon_size = 100
