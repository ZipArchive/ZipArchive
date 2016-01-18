//
//  ViewController.swift
//  SwiftExample
//
//  Created by Sean Soper on 10/23/15.
//
//

import UIKit

#if UseCarthage
    import ZipArchive
#else
    import SSZipArchive
#endif

class ViewController: UIViewController {

    @IBOutlet weak var zipButton: UIButton!
    @IBOutlet weak var unzipButton: UIButton!
    @IBOutlet weak var resetButton: UIButton!

    @IBOutlet weak var file1: UILabel!
    @IBOutlet weak var file2: UILabel!
    @IBOutlet weak var file3: UILabel!

    var zipPath: String?

    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view, typically from a nib.
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }

    // MARK: IBAction

    @IBAction func zipPressed(_: UIButton) {
        let sampleDataPath = NSBundle.mainBundle().bundleURL.URLByAppendingPathComponent("Sample Data").path
        zipPath = tempZipPath()

        let success = SSZipArchive.createZipFileAtPath(zipPath, withContentsOfDirectory: sampleDataPath)
        if success {
            unzipButton.enabled = true
            zipButton.enabled = false
        }
    }

    @IBAction func unzipPressed(_: UIButton) {
        guard let zipPath = self.zipPath else {
            return
        }

        guard let unzipPath = tempUnzipPath() else {
            return
        }

        let success = SSZipArchive.unzipFileAtPath(zipPath, toDestination: unzipPath)
        if !success {
            return
        }

        var items: [String]
        do {
            items = try NSFileManager.defaultManager().contentsOfDirectoryAtPath(unzipPath)
        } catch {
            return
        }

        for (index, item) in items.enumerate() {
            switch index {
            case 0:
                file1.text = item
            case 1:
                file2.text = item
            case 2:
                file3.text = item
            default:
                print("Went beyond index of assumed files")
            }
        }

        unzipButton.enabled = false
        resetButton.enabled = true
    }

    @IBAction func resetPressed(_: UIButton) {
        file1.text = ""
        file2.text = ""
        file3.text = ""
        zipButton.enabled = true
        unzipButton.enabled = false
        resetButton.enabled = false
    }

    // MARK: Private

    func tempZipPath() -> String {
        var path = NSSearchPathForDirectoriesInDomains(.CachesDirectory, .UserDomainMask, true)[0]
        path += "/\(NSUUID().UUIDString).zip"
        return path
    }

    func tempUnzipPath() -> String? {
        var path = NSSearchPathForDirectoriesInDomains(.CachesDirectory, .UserDomainMask, true)[0]
        path += "/\(NSUUID().UUIDString)"
        let url = NSURL(fileURLWithPath: path)

        do {
            try NSFileManager.defaultManager().createDirectoryAtURL(url, withIntermediateDirectories: true, attributes: nil)
        } catch {
            return nil
        }

        if let path = url.path {
            return path
        }

        return nil
    }
    
}
