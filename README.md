# pngimp
A simple PNG importer. Work in progress. MIT Licensed.
## To Do:
### Core Goals:
- [x] Import 8 bit RGB/RGBA PNGs
    - [x] Read Header
    - [x] Read and Concatenate Data Blocks
    - [x] Inflate Data
    - [ ] Unfilter Data
    - [ ] De-Interlace Data
### Future Goals:
- [ ] Support color type 0 (Grayscale)
- [ ] Support color type 3 (Palletized)
- [ ] Support color type 4 (Grayscale with Alpha)
- [ ] Read gAMA chunk
