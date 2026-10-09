# Calibration tools

The width, brightness and tail-length claims in the Insight panel are predicted by small models in
`src/PerceptionProfile.cpp` (`widthChangeFor`, `brightnessShiftFor`, `reverbTailFor`, `echoTailFor`).
They were fitted to audio rendered through the real plugin, not guessed.

## Re-measuring

```
cmake -B build -DECHOPSYCHFX_BUILD_TESTS=ON
cmake --build build --target EchoPsychFXMeasure
build/EchoPsychFXMeasure_artefacts/Release/EchoPsychFXMeasure 1 400 > data.csv     # 400 random settings
build/EchoPsychFXMeasure_artefacts/Release/EchoPsychFXMeasure 0 0 presets > presets.csv
```

The first row (printed when the seed is 0 and not in preset mode) names the columns. Each row holds the inputs the models use (`c.*` fields of `SoundCharacter`) and the measured outcome:

| Column | Measured how |
| --- | --- |
| `dSideMid` | Side/Mid level in dB after minus before, on a half-correlated noise source |
| `dHF` | change in high-band (4-12 kHz) minus low-band (100-500 Hz) level |
| `tail` | seconds until a noise burst falls 40 dB below its peak |
| `motion` | variation of the side/mid ratio over 100 ms windows |

Refit the coefficients against these columns (least squares for brightness, a decorrelation
product model for width), then check the **held-out** error before editing the constants. When the
models were last fitted: width error 0.36 dB on the 31 presets, brightness about 1.3 dB, tail 0.39 s.

## Guarding the wording

`tests/DescriptionConsistencyTest.cpp` applies every factory preset and 20,000 random settings and
fails if any description contains a contradiction (for example "close and dry" together with "long,
cascading echo trails"). Add a word pair to its `clash` list whenever a new chip or phrase is added.

```
cmake --build build --target EchoPsychFXTests && ctest --test-dir build
```
