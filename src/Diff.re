let redPixel = (255, 0, 0, 255);

let maxYIQPossibleDelta = 35215.;

type diffVariant('a) =
  | Layout
  | Pixel(('a, int));

module MakeDiff = (IO: ImageIO.ImageIO) => {
  let compare =
      (
        base: ImageIO.img('a),
        comp: ImageIO.img('a),
        ~threshold=0.1,
        ~diffImage=false,
        (),
      ) => {
    let diffCount = ref(0);
    let diff = base;

    let maxDelta = maxYIQPossibleDelta *. threshold ** 2.;

    let countDifference = (x, y) => {
      diffCount := diffCount^ + 1;
      diff |> IO.setImgColor(x, y, redPixel);
    };

    for (y in 0 to base.height - 1) {
      let row = IO.readRow(base, y);
      let row2 = IO.readRow(comp, y);

      for (x in 0 to base.width - 1) {
        if (x >= comp.width || y >= comp.height) {
          let (_r, _g, _b, a) = IO.readImgColor(x, row, base);
          if (a != 0) {
            countDifference(x, y);
          };
        } else {
          let (r, g, b, a) = IO.readImgColor(x, row, base); /* IO.readImgColor(x, y, base); */
          let (r1, g1, b1, a1) = IO.readImgColor(x, row2, comp); /* IO.readImgColor(x, y, comp); */

          if (r != r1 || g != g1 || b != b1 || a != a1) {
            let delta =
              ColorDelta.calculatePixelColorDelta(
                (r, g, b, a),
                (r1, g1, b1, a1),
              );

            if (delta > maxDelta) {
              countDifference(x, y);
            };
          };
        };
      };
    };
    (diff, diffCount^);
  };

  let diff =
      (
        base: ImageIO.img('a),
        comp: ImageIO.img('a),
        ~threshold=0.1,
        ~diffImage=false,
        ~failOnLayoutChange=true,
        (),
      ) =>
    if (failOnLayoutChange == true
        && base.width != comp.width
        && base.height != comp.height) {
      Layout;
    } else {
      let diffResult = compare(base, comp, ~threshold, ~diffImage, ());
      Pixel(diffResult);
    };
};