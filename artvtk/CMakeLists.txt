# basic source code CMakeLists.txt

art_make( MODULE_LIBRARIES
            vtkPVPythonCatalyst
            artvtk_artvtk
          SERVICE_LIBRARIES
            vtkPVPythonCatalyst
            artvtk_artvtk
          )

install_headers()
install_source()
install_fhicl() 
