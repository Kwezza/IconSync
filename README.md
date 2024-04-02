<!DOCTYPE html>
<html lang="en">
    <body>
        <h1>Icon Sync</h1>
        <p>A common problem I've had when extracting numerous WHDLoad archives, particularly when using the companion program WHDArchiveExtractor, is ending up with folders that have missing icons or icons that don't match my preferred colour scheme. This simple command-line program solves that by allowing you to copy your custom icons into those folders.</p> 
        <h2>How It Works</h2>
        <ul>
            <li>Designed primarily for use after running WHDArchiveExtractor to extract WHDLoad archives, but can be used with any folder</li>
            <li>Requires two folders:</li>
          <ul>
            <li>A target folder containing the folders you want to replace or create icons for</li>
            <li>A source icon folder with your custom icon set (.info files).</li>
            </ul>
            <li>The icon set should include icons for A-Z (A.info - Z.info) and 0 (0.info) for numbered folders.</li>
            <li>You can also include other custom icons for specific folders like Games.info, Demos.info, AGA.info, etc.</li>
          <li>If a folder doesn't match any icons in the source, a default "Blank.info" icon is copied and renamed.</li>
          <li>Skips folders containing .slave files to avoid changing the look made by the WHDLoad game author.</li>
        </ul>
            <h2>Program Features</h2>
            <ul>
                <li>DeleteGameFolderIcons: Removes existing icons for game folders (with .slave files inside) and replaces them with the "Blank.info" icon for a uniform look.</li>
                <li>DeleteExistingIcons: Deletes icons in the target folder if a matching icon exists in the source folder before replacing them.</li>
            </ul>
                  <h2>Important Notes</h2>
            <ul>
                <li>You need to provide your own custom icon set - it's not (yet) included with the program.</li>
                <li>If source icons have been "snapshotted" (locked positions), their positions will be replicated when copied, which could lead to misalignment.  Icons should be unsnapshotted.  I've found that sometimes Workbench won't allow you to unshapshot a folder icon unless you have an actual folder to go with it.</li>
              <li>Don't snapshot the "default.info" icon to avoid clustering all game folders together one on top of each other.</li>
              <li>This program makes changes to your icons, so make sure you can revert if needed.</li>
            </ul>
            <h2>Usage</h2>
        <pre><code>$ IconSync <target_directory> <icon_source_directory> [-DeleteGameFolderIcons] [-DeleteExistingIcons];</code></pre>
        <p>By using this program after extracting WHDLoad archives with WHDArchiveExtractor, you can easily get all your extracted folders to look consistent with your preferred icon theme.</p>
            <h2>Building</h2>
        <p>To compile the program, use an Amiga C compiler, such as SAS/C, with the provided source code.</p>
            <h2>Disclaimer</h2>
        <p>Before using this program, please make sure to backup any important data or files. The creators and contributors of this software are not responsible for any data loss or damage that may occur as a result of using this program.</p>
            <h2>License</h2>
        <p>This project is licensed under the <a href="https://opensource.org/licenses/MIT">MIT License</a>.</p>
    </body>
</html>

