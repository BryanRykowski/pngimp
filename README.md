# pngimp
A single-header PNG importer. Work in progress. MIT Licensed.
## To Do:
### Core Goals:
- [ ] Import 8/16 bit Grayscale/RGB/RGBA PNGs
    - [x] Read Header
    - [x] Read and Concatenate Data Blocks
    - [ ] Inflate Data
    - [ ] Unfilter Data
    - [ ] De-Interlace Data
### Future Goals:
- [ ] Support grayscale bit depths less than 8
- [ ] Support color type 3 (Palletized)
- [ ] Support color type 4 (Grayscale with Alpha)
- [ ] Read gAMA chunk