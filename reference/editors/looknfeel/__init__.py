##############################################################################
#   created:    25th June 2014
#   author:     Lukas E Meindl
##############################################################################
##############################################################################
#   CEED - Unified CEGUI asset editor
#
#   Copyright (C) 2011-2014   Martin Preisler <martin@preisler.me>
#                             and contributing authors (see AUTHORS file)
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
##############################################################################

from ceed import editors

import ceed.compatibility.looknfeel as looknfeel_compatibility

from ceed.editors.looknfeel.tabbed_editor import LookNFeelTabbedEditor

class LookNFeelTabbedEditorFactory(editors.TabbedEditorFactory):
    def getFileExtensions(self):
        extensions = looknfeel_compatibility.manager.getAllPossibleExtensions()
        return extensions

    def canEditFile(self, filePath):
        extensions = self.getFileExtensions()

        for extension in extensions:
            if filePath.endswith("." + extension):
                return True

        return False

    def create(self, filePath):
        return LookNFeelTabbedEditor(filePath)
