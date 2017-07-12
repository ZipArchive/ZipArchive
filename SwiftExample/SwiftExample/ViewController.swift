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

    @IBOutlet weak var passwordField: UITextField!
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
        
        file1.text = ""
        file2.text = ""
        file3.text = ""
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }

    // MARK: IBAction

    @IBAction func zipPressed(_: UIButton) {
        let sampleDataPath = Bundle.main.bundleURL.appendingPathComponent("Sample Data").path
        zipPath = tempZipPath()
        let password = passwordField.text

        let success = SSZipArchive.createZipFile(atPath: zipPath!, withContentsOfDirectory: sampleDataPath, withPassword: password?.isEmpty == false ? password : nil)
        if success {
            unzipButton.isEnabled = true
            zipButton.isEnabled = false
        }
        resetButton.isEnabled = true
    }

    @IBAction func unzipPressed(_: UIButton) {
        guard let zipPath = self.zipPath else {
            return
        }

        guard let unzipPath = tempUnzipPath() else {
            return
        }

        let password = passwordField.text
        let success: Void? = try? SSZipArchive.unzipFile(atPath: zipPath, toDestination: unzipPath, overwrite: true, password: password?.isEmpty == false ? password : nil)
        if success == nil {
            print("No success")
            return
        }

        var items: [String]
        do {
            items = try FileManager.default.contentsOfDirectory(atPath: unzipPath)
        } catch {
            return
        }

        for (index, item) in items.enumerated() {
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

        unzipButton.isEnabled = false
    }

    @IBAction func resetPressed(_: UIButton) {
        file1.text = ""
        file2.text = ""
        file3.text = ""
        zipButton.isEnabled = true
        unzipButton.isEnabled = false
        resetButton.isEnabled = false
    }

    // MARK: Private

    func tempZipPath() -> String {
        var path = NSSearchPathForDirectoriesInDomains(.cachesDirectory, .userDomainMask, true)[0]
        path += "/\(UUID().uuidString).zip"
        return path
    }

    func tempUnzipPath() -> String? {
        var path = NSSearchPathForDirectoriesInDomains(.cachesDirectory, .userDomainMask, true)[0]
        path += "/\(UUID().uuidString)"
        let url = URL(fileURLWithPath: path)

        do {
            try FileManager.default.createDirectory(at: url, withIntermediateDirectories: true, attributes: nil)
        } catch {
            return nil
        }


        return url.path
    }
    
}
